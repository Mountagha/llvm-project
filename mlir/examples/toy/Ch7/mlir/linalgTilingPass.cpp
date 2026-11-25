//====- linalgTilingPass.cpp - tiling linalg operations for better locality. --===//
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

struct LinalgTilingPass
    : public PassWrapper<LinalgTilingPass, OperationPass<func::FuncOp>> {
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(LinalgTilingPass)
    StringRef getArgument() const override { return "toy-linalg-tile"; }

    // Constructor that accepts global CLI options
    LinalgTilingPass(ArrayRef<int64_t> userTilesSizes)
        : customTileSizes(userTilesSizes.begin(), userTilesSizes.end()) {}

    void getDependentDialects(DialectRegistry &registry) const override {
        registry.insert<scf::SCFDialect,
                        mlir::affine::AffineDialect,
                        mlir::tensor::TensorDialect>();

        // Register the TilingInterface implementation for Linalg ops.
        mlir::linalg::registerTilingInterfaceExternalModels(registry);
    }

    void runOnOperation() final;
private:
    // used to distinguish between a map and a reduce. Fragile but will do for now.
    bool hasOnlyParallelIterators(linalg::GenericOp op);
    SmallVector<int64_t> getDefaultTileSizes(linalg::LinalgOp op);
    SmallVector<int64_t, 3> customTileSizes;
};
} // namespace

bool LinalgTilingPass::hasOnlyParallelIterators(linalg::GenericOp op) {
    return llvm::all_of(
        op.getIteratorTypesArray(),
        [](utils::IteratorType type) {
            return type == utils::IteratorType::parallel;
        }
    );
}

SmallVector<int64_t> LinalgTilingPass::getDefaultTileSizes(linalg::LinalgOp op) {
    SmallVector<int64_t> tileSizes;
    for (auto type : op.getIteratorTypesArray()) {
        if (type == utils::IteratorType::parallel) {
            tileSizes.push_back(32);
        } else {
            tileSizes.push_back(0);
        }
    }
    return tileSizes;
}

void LinalgTilingPass::runOnOperation() {
    
    func::FuncOp funcOp = getOperation();
    MLIRContext *context = &getContext();
    IRRewriter rewriter(context);

    // walk the function to find all linalg.matmul operations
    SmallVector<TilingInterface> ops;
    funcOp.walk([&](Operation *op) {
        bool isMatmul = llvm::isa<linalg::MatmulOp>(op);

        bool isMapLinalg = false;
        if (auto generic = dyn_cast<linalg::GenericOp>(op)) {
            isMapLinalg = hasOnlyParallelIterators(generic);
        }
        
        if (!isMatmul && !isMapLinalg) 
            return;
        if (auto tilableOp = dyn_cast<TilingInterface>(op)) {
            ops.push_back(tilableOp);
        }

    });


    // Now apply tiling to each matmul/map found.
    for (TilingInterface tilingOp : ops) {

        Operation *op = tilingOp.getOperation();

        auto linalgOp = cast<linalg::LinalgOp>(op);

        SmallVector<int64_t> tileSizes;
        if (customTileSizes.empty()) {
            tileSizes = getDefaultTileSizes(linalgOp);
        } else {
            tileSizes.assign(
                customTileSizes.begin(),
                customTileSizes.end()
            );
        }
        if (tileSizes.size() != linalgOp.getNumLoops()) {
            op->emitError("tile sizes must match loop rank");
            signalPassFailure();
            return;
        }

        rewriter.setInsertionPoint(tilingOp);

        scf::SCFTilingOptions options;
        options.setTileSizes(
            mlir::getAsIndexOpFoldResult(context, tileSizes)
        );

        // Perform the tiling transformation using SCF loops.
        auto tiled = scf::tileUsingSCF(
            rewriter,
            tilingOp,
            options
        );

        if (failed(tiled)) {
            tilingOp.emitError("failed to tile op");
            return signalPassFailure();
        }

        // Replace the original matmul with the tiled results
        rewriter.replaceOp(tilingOp, tiled->mergeResult.replacements);
    }

}

/// Create a pass for tiling linalg::MatmulOp.
std::unique_ptr<mlir::Pass> mlir::toy::createLinalgTilingPass(ArrayRef<int64_t> tilesSizes) {
    return std::make_unique<LinalgTilingPass>(tilesSizes);
}