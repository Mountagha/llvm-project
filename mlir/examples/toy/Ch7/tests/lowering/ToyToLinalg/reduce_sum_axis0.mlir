
 // RUN: toyc-ch7 %s -emit=mlir-linalg 2>&1 | FileCheck %s

toy.func @main() {
  %0 = toy.constant dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]> : tensor<2x3xf64>

  %1 = "toy.reduce_sum"(%0) {axis = 0 : i64}
    : (tensor<2x3xf64>) -> tensor<3xf64>

  toy.print %1 : tensor<3xf64>
  toy.return
}

// CHECK: linalg.fill
// CHECK: linalg.generic
// CHECK: iterator_types = ["reduction", "parallel"]
// CHECK-NOT: toy.reduce_sum
