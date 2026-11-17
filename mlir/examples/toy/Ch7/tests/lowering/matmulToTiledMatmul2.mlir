
 // RUN: mlir-opt %s -toy-matmul-tile | FileCheck %s

module {
  func.func @matmul(
      %A: tensor<70x70xf64>,
      %B: tensor<70x70xf64>
  ) -> tensor<70x70xf64> {

    %empty = tensor.empty() : tensor<70x70xf64>
    %zero = arith.constant 0.0 : f64

    %init = linalg.fill
      ins(%zero : f64)
      outs(%empty : tensor<70x70xf64>)
      -> tensor<70x70xf64>

    %result = linalg.matmul
      ins(%A, %B : tensor<70x70xf64>, tensor<70x70xf64>)
      outs(%init : tensor<70x70xf64>)
      -> tensor<70x70xf64>

    return %result : tensor<70x70xf64>
  }
}

// CHECK: scf.for
// CHECK: scf.for
// CHECK: tensor.extract_slice
// CHECK: tensor<32x64xf64>
// CHECK: tensor<64x32xf64>
// CHECK: linalg.matmul
// CHECK: tensor.insert_slice
