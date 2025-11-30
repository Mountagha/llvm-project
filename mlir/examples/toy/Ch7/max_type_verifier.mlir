// RUN: toyc-ch7 %s -emit=mlir 2>&1 | FileCheck %s

module {
  toy.func @bad_max() -> tensor<2x2xf64> {
    // 2x2 tensor
    %0 = toy.constant dense<[[1.0, 2.0], [3.0, 4.0]]> : tensor<2x2xf64>

    // 3x3 tensor
    %1 = toy.constant dense<[[1.0, 2.0, 3.0],
                             [4.0, 5.0, 6.0],
                             [7.0, 8.0, 9.0]]> : tensor<3x3xf64>

    // lhs: 2x2, rhs: 3x3 → should trip your verify()
    %2 = "toy.max"(%0, %1)
        : (tensor<2x2xf64>, tensor<3x3xf64>) -> tensor<2x2xf64>

    toy.return %2 : tensor<2x2xf64>
  }
  // CHECK: error: 'toy.max' op requires all operands to have the same type
}
