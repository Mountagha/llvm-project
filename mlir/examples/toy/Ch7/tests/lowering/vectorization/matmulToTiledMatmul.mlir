
// RUN: toyc-ch7 -x=mlir -emit=toy-linalg-vectorize %s 2>&1 | FileCheck %s --implicit-check-not=linalg.matmul --implicit-check-not=affine.min

// CHECK-LABEL: func.func @matmul(

// CHECK: linalg.fill

// CHECK: scf.for
// CHECK: scf.for

// Extract A, B and C tiles.
// CHECK: tensor.extract_slice
// CHECK-SAME: to tensor<32x64xf64>

// CHECK: tensor.extract_slice
// CHECK-SAME: to tensor<64x32xf64>

// CHECK: tensor.extract_slice
// CHECK-SAME: to tensor<32x32xf64>

// Read and broadcast A.
// CHECK: %[[A:.*]] = vector.transfer_read
// CHECK-SAME: permutation_map = affine_map<(d0, d1) -> (d0, 0, d1)>
// CHECK-SAME: vector<32x32x64xf64>

// Read and broadcast B.
// CHECK: %[[B:.*]] = vector.transfer_read
// CHECK-SAME: permutation_map = affine_map<(d0, d1) -> (0, d1, d0)>
// CHECK-SAME: vector<32x32x64xf64>

// Read the accumulator.
// CHECK: %[[ACC:.*]] = vector.transfer_read
// CHECK-SAME: vector<32x32xf64>

// Elementwise multiplication.
// CHECK: %[[PRODUCT:.*]] = arith.mulf %[[A]], %[[B]]
// CHECK-SAME: vector<32x32x64xf64>

// Reduction along K.
// CHECK: %[[REDUCED:.*]] = vector.multi_reduction <add>, %[[PRODUCT]], %[[ACC]] [2]
// CHECK-SAME: vector<32x32x64xf64> to vector<32x32xf64>

// Write the result.
// CHECK: %[[WRITE:.*]] = vector.transfer_write %[[REDUCED]]
// CHECK-SAME: tensor<32x32xf64>

// CHECK: %[[INSERT:.*]] = tensor.insert_slice %[[WRITE]]
// CHECK: scf.yield %[[INSERT]]
// CHECK: scf.yield
// CHECK: return


module {
  func.func @matmul(
      %A: tensor<64x64xf64>,
      %B: tensor<64x64xf64>
  ) -> tensor<64x64xf64> {

    %empty = tensor.empty() : tensor<64x64xf64>
    %zero = arith.constant 0.0 : f64

    %init = linalg.fill
      ins(%zero : f64)
      outs(%empty : tensor<64x64xf64>)
      -> tensor<64x64xf64>

    %result = linalg.matmul
      ins(%A, %B : tensor<64x64xf64>, tensor<64x64xf64>)
      outs(%init : tensor<64x64xf64>)
      -> tensor<64x64xf64>

    return %result : tensor<64x64xf64>
  }
}

// CHECK: scf.for
// CHECK: scf.for
