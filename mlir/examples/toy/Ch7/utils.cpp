//===- utils.cpp - Shape Inference ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements some utilities functions used by different parts 
// of the code
//
//===----------------------------------------------------------------------===//

#include "toy/utils.h"
#include "mlir/Support/LogicalResult.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include <algorithm>

namespace mlir {
namespace toy {

FailureOr<llvm::SmallVector<int64_t>>
computeBroadcastedShape(llvm::ArrayRef<int64_t> a,
                                                llvm::ArrayRef<int64_t> b) {
    int ra = static_cast<int>(a.size());
    int rb = static_cast<int>(b.size());
    int r = std::max(ra, rb);

    llvm::SmallVector<int64_t> out(r, 1);

    for (int i = 0; i < r; ++i) {
        int64_t da = (i < ra) ? a[ra - 1 - i] : 1;
        int64_t db = (i < rb) ? b[rb - 1 - i] : 1;

        if (da != db && da != 1 && db != 1)
            return mlir::failure();

        out[r - 1 - i] = std::max(da, db);
    }
    return out;
}

} // namespace toy
} // namespace mlir