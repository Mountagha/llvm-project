
 // RUN: toyc-ch7 %s -emit=mlir-linalg 2>&1 | FileCheck %s

toy.func @main() {
  %0 = toy.constant dense<[[1.0, 5.0], [3.0, 2.0], [4.0, 6.0]]> : tensor<3x2xf64>
  %1 = toy.constant dense<[[0.0, 10.0], [4.0, -1.0]]> : tensor<2x2xf64>
  %2 = toy.matmul %0, %1 : tensor<3x2xf64>, tensor<2x2xf64> -> tensor<3x2xf64>
  toy.print %2 : tensor<3x2xf64>
  toy.return
}

// CHECK: linalg.fill
// CHECK: linalg.matmul
// CHECK-NOT: toy.matmul
