//====- vectorizePass.cpp - Vectorize linalg ops --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a vectorization pass for the suited linalg ops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/TileUsingInterface.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/Linalg/Transforms/TilingInterfaceImpl.h"
#include "mlir/Interfaces/TilingInterface.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "toy/Dialect.h"
#include "toy/Passes.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Casting.h"

using namespace mlir;

namespace {

    struct VectorizePass
        : public PassWrapper<VectorizePass, OperationPass<func::FuncOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(VectorizePass)
        StringRef getArgument() const override { return "toy-linalg-vectorize"; }
        StringRef getDescription() const override {
            return "Apply vectorization to linalg suitable ops";
        }

        void runOnOperation() final;
    };
} // namespace.

void VectorizePass::runOnOperation() {
    func::FuncOp funcOP = getOperation();
    MLIRContext *ctx = &getContext();
    IRRewriter rewriter(ctx);

    // Collect ops suited for vectorization. For now matmul/map

    SmallVector<linalg::LinalgOp, 2> toVectorize;
    funcOP.walk([&](linalg::LinalgOp op) {
        bool isMatmul = isa<linalg::MatmulOp>(op.getOperation());

        bool isMapLike = false;

        if (auto genericOp = dyn_cast<linalg::GenericOp>(op.getOperation())) {
            isMapLike = llvm::all_of(
                genericOp.getIteratorTypesArray(),
                [](utils::IteratorType type) {
                    return type == utils::IteratorType::parallel;
                }
            );
        }

        if (!isMatmul || !isMapLike)
            return;
        
        if (op.hasDynamicShape())
            return;

        toVectorize.push_back(op);
    });

    for(linalg::LinalgOp op : toVectorize) {
        rewriter.setInsertionPoint(op);

        if(failed(linalg::vectorize(
            rewriter,
            op.getOperation()))) {
                op->emitError("failed to vectorize operation.");
                signalPassFailure();
                return;
        }

    }

}

std::unique_ptr<mlir::Pass> mlir::toy::createVectorizePass() {
    return std::make_unique<VectorizePass>();
}