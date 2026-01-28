//===- utils.h - Interface definitions for some utilities -=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains some utlity functions used by different part of the code.
//
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_TUTORIAL_TOY_UTILS_H_
#define MLIR_TUTORIAL_TOY_UTILS_H_

#include "mlir/IR/OpDefinition.h"

namespace mlir {
namespace toy {

static mlir::FailureOr<llvm::SmallVector<int64_t>>
computeBroadcastedShape(llvm::ArrayRef<int64_t> a, llvm::ArrayRef<int64_t> b);

} // namespace toy
} // namespace mlir

#endif // MLIR_TUTORIAL_TOY_SHAPEINFERENCEINTERFACE_H_
