//===- Passes.h - Toy Passes Definition -----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file exposes the entry points to create compiler passes for Toy.
//
//===----------------------------------------------------------------------===//

#ifndef TOY_PASSES_H
#define TOY_PASSES_H

#include <memory>
#include <llvm/ADT/ArrayRef.h>
#include <cstdint>

namespace mlir {
class Pass;

namespace toy {
std::unique_ptr<Pass> createShapeInferencePass();

/// Create a pass for lowering to operations in the `Affine` and `Std` dialects,
/// for a subset of the Toy IR (e.g. matmul).
std::unique_ptr<mlir::Pass> createLowerToAffinePass();

/// Create a pass for lowering operations the remaining `Toy` operations, as
/// well as `Affine` and `Std`, to the LLVM dialect for codegen.
std::unique_ptr<mlir::Pass> createLowerToLLVMPass();

/// Create a pass for lowering operations that have a perfect loop
/// nest (e.g. matmul) into `Linalg` + `Arith`.
std::unique_ptr<mlir::Pass> createLowerToLinalgPass();

/// Create a pass for tiling selected Linalg operations.
/// By default, tiles parallel dimensions and leaves reductions untiled.
std::unique_ptr<mlir::Pass> createLinalgTilingPass(ArrayRef<int64_t> tilesSizes = {});

/// Create a pass for peeling loop so that we can get rid of dynamic shape
/// on the tiled linalg ops. PS: to completely get rid of the dynamic shapes
/// we might need to invoke canonalization/cse passes after the peeling
std::unique_ptr<mlir::Pass> createLoopPeelingPass();

/// Create a pass for vectorizing linalg ops.
std::unique_ptr<mlir::Pass> createVectorizePass();

} // namespace toy
} // namespace mlir

#endif // TOY_PASSES_H
