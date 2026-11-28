//====- loopPeelingPass.cpp - Loop Peeling suited linalg ops. --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a LoopPeeling pass to facilitate vectorization.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/TileUsingInterface.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
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

    struct LoopPeelingPass
        : public PassWrapper<LoopPeelingPass, OperationPass<func::FuncOp>> {
        MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(LoopPeelingPass)
        StringRef getArgument() const override { return "toy-linalg-peeling"; }
        StringRef getDescription() const override {
            return "Peel scf.for loops for staic tiles to facilitate vectorization";
        }

        void runOnOperation() final;
    };
} // namespace.

void LoopPeelingPass::runOnOperation() {
    func::FuncOp funcOP = getOperation();
    MLIRContext *ctx = &getContext();
    IRRewriter rewriter(ctx);

    // Collect Targets scf.for loops

    // Collect loops from innermost to outermost.
    // Peeling an outer loop clones its body, so nested
    // loops should already be peeled before their
    // enclosing loop is transformed.
    SmallVector<scf::ForOp, 2> loopsToPeel;
    funcOP.walk<WalkOrder::PostOrder>([&](scf::ForOp forOp) {
        loopsToPeel.push_back(forOp);
    });

    // Apply the peeling transformation.
    for (scf::ForOp loop: loopsToPeel) {
        rewriter.setInsertionPoint(loop);

        scf::ForOp partialIterationLoop;
        if(failed(scf::peelForLoopAndSimplifyBounds(
            rewriter,
            loop,
            partialIterationLoop
        ))) {
            // Later handle 
            continue;
        }

    }

}

std::unique_ptr<mlir::Pass> mlir::toy::createLoopPeelingPass() {
    return std::make_unique<LoopPeelingPass>();
}