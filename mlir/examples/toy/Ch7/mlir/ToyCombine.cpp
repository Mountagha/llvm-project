//===- ToyCombine.cpp - Toy High Level Optimizer --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a set of simple combiners for optimizing operations in
// the Toy dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Value.h"
#include "toy/Dialect.h"
#include "llvm/Support/Casting.h"
#include <cstddef>
using namespace mlir;
using namespace toy;

namespace {
/// Include the patterns defined in the Declarative Rewrite framework.
#include "ToyCombine.inc"
} // namespace

/// Fold constants.
OpFoldResult ConstantOp::fold(FoldAdaptor adaptor) { return getValue(); }

/// Fold struct constants.
OpFoldResult StructConstantOp::fold(FoldAdaptor adaptor) { return getValue(); }

/// Fold simple struct access operations that access into a constant.
OpFoldResult StructAccessOp::fold(FoldAdaptor adaptor) {
  auto structAttr =
      llvm::dyn_cast_if_present<mlir::ArrayAttr>(adaptor.getInput());
  if (!structAttr)
    return nullptr;

  size_t elementIndex = getIndex();
  return structAttr[elementIndex];
}


OpFoldResult ReduceSumOp::fold(FoldAdaptor adaptor) {
  // 1) Fold only if operand is a constant dense FP tensor.
  auto inputAttr =
      llvm::dyn_cast_if_present<DenseFPElementsAttr>(adaptor.getInput());
  if (!inputAttr)
    return nullptr;

  // 2) Require ranked input and ranked result.
  auto inputType = llvm::dyn_cast<RankedTensorType>(inputAttr.getType());
  auto resultType = llvm::dyn_cast<RankedTensorType>(getResult().getType());
  if (!inputType || !resultType)
    return nullptr;

  // 3) Validate axis.
  int64_t rank = inputType.getRank();
  int64_t axis = getAxis();
  if (rank <= 0 || axis < 0 || axis >= rank)
    return nullptr;

  // 4) Build shape vectors.
  llvm::SmallVector<int64_t> inputShape(inputType.getShape().begin(),
                                        inputType.getShape().end());
  llvm::SmallVector<int64_t> resultShape(resultType.getShape().begin(),
                                         resultType.getShape().end());

  // 5) Helper to compute row-major strides.
  auto computeStrides = [](ArrayRef<int64_t> shape) {
    llvm::SmallVector<int64_t> strides(shape.size(), 1);
    for (int64_t i = static_cast<int64_t>(shape.size()) - 2; i >= 0; --i)
      strides[i] = strides[i + 1] * shape[i + 1];
    return strides;
  };

  auto inputStrides = computeStrides(inputShape);
  auto resultStrides = computeStrides(resultShape);

  // 6) Accumulate reduced values.
  llvm::SmallVector<double> reduced(resultType.getNumElements(), 0.0);
  auto inputValues = llvm::to_vector(inputAttr.getValues<double>());

  for (int64_t linearIndex = 0, end = inputType.getNumElements();
       linearIndex < end; ++linearIndex) {
    int64_t remaining = linearIndex;
    int64_t resultLinearIndex = 0;
    int64_t resultDim = 0;

    for (int64_t dim = 0; dim < rank; ++dim) {
      int64_t coord = remaining / inputStrides[dim];
      remaining %= inputStrides[dim];

      if (dim == axis)
        continue;

      // If result is scalar, resultStrides is empty and index stays 0.
      if (!resultStrides.empty())
        resultLinearIndex += coord * resultStrides[resultDim];
      ++resultDim;
    }

    reduced[resultLinearIndex] += inputValues[linearIndex];
  }

  // 7) Return a constant attribute replacing this op.
  return DenseElementsAttr::get(resultType, ArrayRef<double>(reduced));
}


/// This is an example of a c++ rewrite pattern for the TransposeOp. It
/// optimizes the following scenario: transpose(transpose(x)) -> x
struct SimplifyRedundantTranspose : public mlir::OpRewritePattern<TransposeOp> {
  /// We register this pattern to match every toy.transpose in the IR.
  /// The "benefit" is used by the framework to order the patterns and process
  /// them in order of profitability.
  SimplifyRedundantTranspose(mlir::MLIRContext *context)
      : OpRewritePattern<TransposeOp>(context, /*benefit=*/1) {}

  /// This method attempts to match a pattern and rewrite it. The rewriter
  /// argument is the orchestrator of the sequence of rewrites. The pattern is
  /// expected to interact with it to perform any changes to the IR from here.
  llvm::LogicalResult
  matchAndRewrite(TransposeOp op,
                  mlir::PatternRewriter &rewriter) const override {
    // Look through the input of the current transpose.
    mlir::Value transposeInput = op.getOperand();
    TransposeOp transposeInputOp = transposeInput.getDefiningOp<TransposeOp>();

    // Input defined by another transpose? If not, no match.
    if (!transposeInputOp)
      return failure();

    // Otherwise, we have a redundant transpose. Use the rewriter.
    rewriter.replaceOp(op, {transposeInputOp.getOperand()});
    return success();
  }
};

// Canonalize relu(relu(x)) -> relu(x)
struct SimplifyNestedRelu: public mlir::OpRewritePattern<ReluOp> {
  SimplifyNestedRelu(mlir::MLIRContext *context)
    : OpRewritePattern<ReluOp>(context, /* benefit */1) {}

  llvm::LogicalResult
  matchAndRewrite(ReluOp op, mlir::PatternRewriter &rewriter) const override {
    mlir::Value reluInput = op.getOperand();
    ReluOp innerReluOp = reluInput.getDefiningOp<ReluOp>();

    if (!innerReluOp) {
      return failure();
    }

    // replace relu(relu(x)) by relu(x)
    rewriter.replaceOp(op, innerReluOp);
    return success();
  }
};


/// Fold neg(neg(x)) -> x
struct SimplifyRedundantNeg : public mlir::OpRewritePattern<NegOp> {
  SimplifyRedundantNeg(mlir::MLIRContext *context)
    : OpRewritePattern<NegOp>(context, /*benefit=1*/1) {}
  
    llvm::LogicalResult
    matchAndRewrite(NegOp op,
                  mlir::PatternRewriter &rewriter) const override {
      NegOp innerNeg = op.getInput().getDefiningOp<toy::NegOp>();
      if(!innerNeg) {
        return mlir::failure();
      }
      
      // neg(neg(x)) -> x
      rewriter.replaceOp(op, innerNeg.getInput());
      return mlir::success();
    }
};

/// Max(x, x) -> x
struct SimplifyMax : public mlir::OpRewritePattern<MaxOp> {
  SimplifyMax(mlir::MLIRContext* context)
    : OpRewritePattern<MaxOp>(context, /*benefit=*/1) {}

  llvm::LogicalResult
  matchAndRewrite(MaxOp op,
                  mlir::PatternRewriter &rewriter) const override {
    mlir::Value lhs = op.getLhs();
    mlir::Value rhs = op.getRhs();
    if (lhs != rhs) {
      return failure();
    }  
    rewriter.replaceOp(op, lhs);
    return success();
  }
};

/// Register our patterns as "canonicalization" patterns on the TransposeOp so
/// that they can be picked up by the Canonicalization framework.
void TransposeOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                              MLIRContext *context) {
  results.add<SimplifyRedundantTranspose>(context);
}

/// Register our patterns as "canonicalization" patterns on the ReshapeOp so
/// that they can be picked up by the Canonicalization framework.
void ReshapeOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                            MLIRContext *context) {
  results.add<ReshapeReshapeOptPattern, RedundantReshapeOptPattern,
              FoldConstantReshapeOptPattern>(context);
}

/// Register our patterns as "canonicalization" patterns on the ReshapeOp so
/// that they can be picked up by the Canonicalization framework.
void NegOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                        MLIRContext *context) {
  results.add<SimplifyRedundantNeg>(context);
}

/// Register our patterns as "canonicalization" patterns on the MaxOp so
/// that they can be picked up by the Canonicalization framework.
void MaxOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                        MLIRContext *context) {
  results.add<SimplifyMax>(context);
}

/// Register our patterns as "canonicalization" patterns on the MaxOp so
/// that they can be picked up by the Canonicalization framework.
void ReluOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                        MLIRContext *context) {
  results.add<SimplifyNestedRelu>(context);
}