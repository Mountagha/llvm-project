
 // RUN: toyc-ch7 %s -emit=mlir-linalg 2>&1 | FileCheck %s

toy.func @main() {
  %0 = toy.constant dense<[[1.0, 2.0], [3.0, 4.0]]> : tensor<2x2xf64>

  %1 = toy.map(%0 : tensor<2x2xf64>) -> tensor<2x2xf64> {
  ^bb0(%x : f64):
    %two = arith.constant 2.0 : f64
    %result = arith.mulf %x, %two : f64
    toy.yield %result : f64
  }

  toy.print %1 : tensor<2x2xf64>
  toy.return
}

// CHECK: linalg.generic
// CHECK: arith.mulf
// CHECK: linalg.yield
// CHECK-NOT: toy.map
