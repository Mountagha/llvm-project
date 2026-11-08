//====- LowerToLinalg.cpp - Partial lowering from Toy to Linalg+Arith --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a partial lowering of Toy operations to the Linalg
// dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/AffineMap.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/TypeID.h"
#include "toy/Dialect.h"
#include "toy/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/Utils/StructuredOpsUtils.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/Support/Casting.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <limits>

using namespace mlir;

//===----------------------------------------------------------------------===//
// ToyToLinalg RewritePatterns
//===----------------------------------------------------------------------===//
namespace {

struct MatMulOpLowering
    : public OpRewritePattern<toy::MatMulOp> {
    
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(
        toy::MatMulOp op,
        PatternRewriter &rewriter) const override {

        Location loc = op.getLoc();
        Value lhs = op.getLhs();
        Value rhs = op.getRhs();

        auto resultTensorType = llvm::cast<RankedTensorType>(op.getResult().getType());

        auto out = rewriter.create<tensor::EmptyOp>(
            loc, resultTensorType, ValueRange{}
        );

        auto elementType = llvm::cast<FloatType>(resultTensorType.getElementType());
        auto zeroAttr = rewriter.getFloatAttr(elementType, 0.0);
        Value zero = rewriter.create<arith::ConstantOp>(loc, zeroAttr);

        auto fillOp = rewriter.create<linalg::FillOp>(
            loc,
            ValueRange { zero },
            ValueRange { out }
        );

        Value initializedOutput = fillOp.getResult(0);

        auto matmulOp = rewriter.create<linalg::MatmulOp>(
            loc,
            resultTensorType,
            ValueRange {lhs, rhs},
            ValueRange { initializedOutput }
        );

        rewriter.replaceOp(op, matmulOp.getResult(0));
        return success();
    }
};

} // namespace

namespace {
struct MapOpLowering:
    public OpRewritePattern<toy::MapOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult
    matchAndRewrite(toy::MapOp op, PatternRewriter& rewriter) const override {
        Location loc = op.getLoc();
        Value input = op.getInput();

        auto resultTensorType = 
            llvm::cast<RankedTensorType>(
                op.getResult().getType()
            );
        
        unsigned rank = resultTensorType.getRank();

        Region &region = op.getBody();

        if (!llvm::hasSingleElement(region))
        return rewriter.notifyMatchFailure(
            op, "expected a single-block map region");

        Block &body = region.front();

        if (body.getNumArguments() != 1 ||
            !llvm::isa<toy::YieldOp>(body.getTerminator())) {
        return rewriter.notifyMatchFailure(
            op, "unexpected map region structure");
        }

        for (Operation &innerOp : body.without_terminator()) {
            if (innerOp.getDialect()->getNamespace() !=
                arith::ArithDialect::getDialectNamespace()) {
                return rewriter.notifyMatchFailure(
                    op, "map body contains an unsupported operation");
            }
        }

        // Create destination tensor.
        Value output = rewriter.create<tensor::EmptyOp>(
            loc, resultTensorType, ValueRange{}
        );

        AffineMap identityMap = 
            rewriter.getMultiDimIdentityMap(rank);
        
        SmallVector<AffineMap> indexingMaps = {
            identityMap,
            identityMap
        };

        SmallVector<utils::IteratorType> iteratorTypes(
            rank, utils::IteratorType::parallel
        );

        auto linalgMapOp = rewriter.create<linalg::GenericOp>(
            loc,
            TypeRange{ resultTensorType },
            ValueRange { input },
            ValueRange { output },
            indexingMaps,
            iteratorTypes,
            [&](OpBuilder &b, Location l, ValueRange args) {
                IRMapping mapping;
                Block &body = op.getRegion().front();
                mapping.map(body.getArgument(0), args[0]);

                // Clone the scalar operations from toy region map into the linalg.generic region.
                for (Operation &innerOp: body.without_terminator()) {
                    // TODO: Ensure that the region contains only scalar operations
                    // legal in the target pipeline. Cloning does not perform lowering.
                    b.clone(innerOp, mapping);
                }

                auto toy_yield = llvm::cast<toy::YieldOp>(body.getTerminator());
                Value linalg_yielded = mapping.lookup(toy_yield.getOperand());

                b.create<linalg::YieldOp>(l, linalg_yielded);

            }
        );

        rewriter.replaceOp(op, linalgMapOp.getResult(0));
        return success();
    }
};

} // namespace.

/// ReduceSumOp Lowering

namespace {
struct ReduceSumOpLowering:
    public OpRewritePattern<toy::ReduceSumOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult
    matchAndRewrite(toy::ReduceSumOp op, PatternRewriter& rewriter) const override {
        Location loc = op.getLoc();
        Value input = op.getInput();

        auto inputType = 
            llvm::cast<RankedTensorType>(input.getType());


        auto resultTensorType =
            llvm::cast<RankedTensorType>(op.getResult().getType());

        int64_t rank = inputType.getRank();
        int64_t reduction_axis = op.getAxis();

        if (reduction_axis < 0 || reduction_axis >= rank)
            return rewriter.notifyMatchFailure(
                op, "invalid reduction axis"
        );

        auto out = rewriter.create<tensor::EmptyOp>(
            loc, resultTensorType, ValueRange{}
        );

        auto elementType = llvm::cast<FloatType>(resultTensorType.getElementType());
        auto zeroAttr = rewriter.getFloatAttr(elementType, 0.0);
        Value zero = rewriter.create<arith::ConstantOp>(loc, zeroAttr);

        auto fillOp = rewriter.create<linalg::FillOp>(
            loc,
            ValueRange { zero },
            ValueRange { out }
        );

        Value initializedOutput = fillOp.getResult(0);

        // the input map is always the full identity map
        AffineMap inputMap = rewriter.getMultiDimIdentityMap(rank);

        // Build outputMap and IteratorsType.
        SmallVector<AffineExpr> outExprs;
        outExprs.reserve(rank - 1);
        
        SmallVector<utils::IteratorType> iteratorTypes;
        iteratorTypes.reserve(rank);

        for(int64_t i = 0; i < rank; ++i) {
            if (i == reduction_axis) {
                // Reduce away this specific axis.
                iteratorTypes.push_back(
                    utils::IteratorType::reduction);
            } else {
                // all other axes remain parallel
                outExprs.push_back(
                    getAffineDimExpr(i, rewriter.getContext()));

                iteratorTypes.push_back(
                    utils::IteratorType::parallel);
            }
        }

        AffineMap outputMap = AffineMap::get(rank, /*symbolCount=*/0, outExprs, rewriter.getContext());

        SmallVector<AffineMap> indexingMaps = {
            inputMap,
            outputMap
        };

        auto reduceSumOp = rewriter.create<linalg::GenericOp>(
            loc,
            TypeRange { resultTensorType },
            ValueRange { input },
            ValueRange { initializedOutput },
            indexingMaps,
            iteratorTypes,
            [&] (OpBuilder &b, Location l, ValueRange args) {
                Value sum = b.create<arith::AddFOp>(l, args[0], args[1]);
                b.create<linalg::YieldOp>(l, sum);
            }
        );

        rewriter.replaceOp(op, reduceSumOp.getResult(0));
        return success();

    }
};

} // namespace

//===----------------------------------------------------------------------===//
// ToyToLinalgLoweringPass
//===----------------------------------------------------------------------===//

/// This is a partial lowering to linalg dialect of the toy operations that have a
/// a perfect loop nest structure (like matmul for example...) while keeping the
/// rest of the code in the Toy dialect.

namespace {
struct ToyToLinalgLoweringPass
    : public PassWrapper<ToyToLinalgLoweringPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ToyToLinalgLoweringPass)
  StringRef getArgument() const override { return "toy-to-linalg"; }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<arith::ArithDialect,
                    tensor::TensorDialect,
                    linalg::LinalgDialect>();
  }
  void runOnOperation() final;
};
} // namespace

void ToyToLinalgLoweringPass::runOnOperation() {
  // The first thing to define is the conversion target. This will define the
  // final target for this lowering.
  MLIRContext *context = &getContext();
  ConversionTarget target(*context);

  // We define the specific operations, or dialects, that are legal targets for
  // this lowering. In our case, we are lowering to a combination of the
  // `linalg`, `Arith`, and `tensor` dialects.
  target.addLegalDialect<BuiltinDialect,
                        linalg::LinalgDialect,
                        arith::ArithDialect,
                        tensor::TensorDialect>();

  // We are only lowering 3 operations for now, thus we are only going to
  // mark those illegals and keep everything else Legal for now.
  target.addLegalDialect<toy::ToyDialect>();
  target.addIllegalOp<
    toy::MatMulOp,
    toy::MapOp,
    toy::ReduceSumOp>();

  // Now that the conversion target has been defined, we just need to provide
  // the set of patterns that will lower the Toy operations.
  RewritePatternSet patterns(context);
  patterns.add<MatMulOpLowering, MapOpLowering, ReduceSumOpLowering>(
      &getContext());

  // With the target and rewrite patterns defined, we can now attempt the
  // conversion. The conversion will signal failure if any of our `illegal`
  // operations were not converted successfully.
  if (failed(
          applyPartialConversion(getOperation(), target, std::move(patterns))))
    signalPassFailure();
}

/// Create a pass for lowering operations in the `Linalg` and `arith` dialects,
/// for a subset of the Toy IR (e.g. matmul).
std::unique_ptr<Pass> mlir::toy::createLowerToLinalgPass() {
  return std::make_unique<ToyToLinalgLoweringPass>();
}