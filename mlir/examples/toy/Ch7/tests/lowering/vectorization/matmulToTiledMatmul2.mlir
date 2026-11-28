
// RUN: toyc-ch7 -x=mlir -emit=toy-linalg-vectorize %s 2>&1 | FileCheck %s --implicit-check-not=linalg.matmul --implicit-check-not=affine.min

// CHECK-LABEL: func.func @matmul(

// CHECK: linalg.fill

// Main tile: 32 x 32.
// CHECK: %[[P0:.*]] = arith.mulf
// CHECK-SAME: vector<32x32x70xf64>

// CHECK: %[[R0:.*]] = vector.multi_reduction <add>, %[[P0]],
// CHECK-SAME: [2] : vector<32x32x70xf64> to vector<32x32xf64>

// CHECK: vector.transfer_write %[[R0]]
// CHECK-SAME: tensor<32x32xf64>

// Right boundary: 32 x 6.
// CHECK: %[[P1:.*]] = arith.mulf
// CHECK-SAME: vector<32x6x70xf64>

// CHECK: %[[R1:.*]] = vector.multi_reduction <add>, %[[P1]],
// CHECK-SAME: [2] : vector<32x6x70xf64> to vector<32x6xf64>

// CHECK: vector.transfer_write %[[R1]]
// CHECK-SAME: tensor<32x6xf64>

// Bottom boundary: 6 x 32.
// CHECK: %[[P2:.*]] = arith.mulf
// CHECK-SAME: vector<6x32x70xf64>

// CHECK: %[[R2:.*]] = vector.multi_reduction <add>, %[[P2]],
// CHECK-SAME: [2] : vector<6x32x70xf64> to vector<6x32xf64>

// CHECK: vector.transfer_write %[[R2]]
// CHECK-SAME: tensor<6x32xf64>

// Bottom-right corner: 6 x 6.
// CHECK: %[[P3:.*]] = arith.mulf
// CHECK-SAME: vector<6x6x70xf64>

// CHECK: %[[R3:.*]] = vector.multi_reduction <add>, %[[P3]],
// CHECK-SAME: [2] : vector<6x6x70xf64> to vector<6x6xf64>

// CHECK: vector.transfer_write %[[R3]]
// CHECK-SAME: tensor<6x6xf64>

// CHECK: return


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

