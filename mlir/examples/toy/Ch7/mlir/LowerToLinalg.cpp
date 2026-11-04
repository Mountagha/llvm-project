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

        auto fillOp = rewriter.create<mlir::linalg::FillOp>(
            loc,
            zero,
            out
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
