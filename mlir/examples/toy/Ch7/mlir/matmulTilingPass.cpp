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
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/TileUsingInterface.h"
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

struct MatMulTilingPass
    : public PassWrapper<MatMulTilingPass, OperationPass<func::FuncOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(MatMulTilingPass)
    StringRef getArgument() const override { return "toy-matmul-tile"; }

    // Constructor that accepts global CLI options
    MatMulTilingPass(ArrayRef<int64_t> userTilesSizes) {
        if (!userTilesSizes.empty()) {
            customTileSizes.assign(userTilesSizes.begin(), userTilesSizes.end());
        } else {
            customTileSizes = {32, 32, 32}; // default fallback.
        }
    }
    void getDependentDialects(DialectRegistry &registry) const override {
        registry.insert<scf::SCFDialect,
                        mlir::affine::AffineDialect,
                        mlir::tensor::TensorDialect>();

        // Register the TilingInterface implementation for Linalg ops.
        mlir::linalg::registerTilingInterfaceExternalModels(registry);
    }

    void runOnOperation() final;
private:
    SmallVector<int64_t, 3> customTileSizes;
};
} // namespace

void MatMulTilingPass::runOnOperation() {
    
    func::FuncOp funcOp = getOperation();
    MLIRContext *context = &getContext();
    IRRewriter rewriter(context);

    // walk the function to find all linalg.matmul operations
    SmallVector<linalg::MatmulOp> matmuls;
    funcOp.walk([&](linalg::MatmulOp op) {
        matmuls.push_back(op);
    });


    // Now apply tiling to each matmul found.
    for (linalg::MatmulOp matmul : matmuls) {
        rewriter.setInsertionPoint(matmul);

        scf::SCFTilingOptions options;
        options.setTileSizes(
            mlir::getAsIndexOpFoldResult(context, customTileSizes)
        );

        // Cast to TilingInterface which linalg.matmul implements
        TilingInterface tilingOp = cast<TilingInterface>(matmul.getOperation());

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
        rewriter.replaceOp(matmul, tiled->mergeResult.replacements);
    }

}

/// Create a pass for tiling linalg::MatmulOp.
std::unique_ptr<mlir::Pass> mlir::toy::createMatMulTilingPass(ArrayRef<int64_t> tilesSizes) {
    return std::make_unique<MatMulTilingPass>(tilesSizes);
}