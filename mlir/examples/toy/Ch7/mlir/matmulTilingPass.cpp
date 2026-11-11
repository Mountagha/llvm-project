//====- LowerToLinalg.cpp - Partial lowering from Toy to Linalg+Arith --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a tiling pass for the linalg matmul op.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/TileUsingInterface.h"
#include "mlir/Interfaces/TilingInterface.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/ArrayRef.h"

namespace {

struct MatMulTilingPass
    : public PassWrapper<MatMulTilingPass, OperationPass<func::FuncOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(MatMulTilingPass)
    StringRef getArgument() const override { return "toy-matmul-tile"; }

    // Constructor that accepts global CLI options
    MatMulTilingPass(ArrayRef<int64_t> userTilesSizes) {
        if (!userTilesSizes.empty()) {
            customTilesSizes.assign(userTilesSizes.begin(), userTilesSizes.end());
        } else {
            customTilesSizes = {32, 32, 32}; // default fallback.
        }
    }
    void getDependentDialects(DialectRegistry &registry) const override {
        registry.insert<scf::SCFDialect>();
    }

    void runOnOperation() final;
private:
    SmallVector<int64_t, 3> customTilesSizes;
};
} // namespace

void MatMulTilingPass::runOnOperation() {
    
    func::FuncOp funcOP = getOperation();
    MLIRContext *context = &getContext();
    IRRewriter rewriter(context);

    // walk the function to find all linalg.matmul operations
    SmallVector<linalg::MatMulOp> matmuls;
    funcOp.walk([&](linalg::MatMulOp op) {
        matmuls.push_back(op);
    });


    // Now apply tiling to each matmul found.
    for (linalg::MatMulOp matmul : matmuls) {
        rewriter.setInsertionPoint(matmul);

        scf::SCFTilingOPtions options;
        options.setTilesSizes(customTilesSizes);

        // Cast to TilingInterface which linalg.matmul implements
        TilingInterface tilingOp = cast<TilingInterface>(matmulOp.getOperation());

        // Perform the tiling transformation using SCF loops.
        auto tiled = scf::tileUsingSCF(
            rewriter,
            tilingOp,
            options
        );

        if (failed(tiled)) {
            matmul.emitError("failed to tile linalg.matmul");
            return signalPassFailure();
        }

        // Replace the original matmul with the tiled results
        rewriter.replaceOp(matmulOp, tiled->replacements);
    }

}

/// Create a pass for tiling linalg::MatMulOp.
std::unique_ptr<Pass> mlir::toy::createMatMulTilingPass(ArrayRef<int64_t> tilesSizes) {
    return std::make_unique<MatMulTilingPass>(tilesSizes);
}