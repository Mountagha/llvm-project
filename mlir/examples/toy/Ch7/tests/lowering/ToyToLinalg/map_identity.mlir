
 // RUN: toyc-ch7 %s -emit=mlir-linalg 2>&1 | FileCheck %s

toy.func @main() {
  %0 = toy.constant dense<[1.0, 2.0, 3.0]> : tensor<3xf64>

  %1 = toy.map(%0 : tensor<3xf64>) -> tensor<3xf64> {
  ^bb0(%x : f64):
    toy.yield %x : f64
  }

  toy.print %1 : tensor<3xf64>
  toy.return
}

// CHECK: linalg.generic
// CHECK: linalg.yield
// CHECK-NOT: toy.map
