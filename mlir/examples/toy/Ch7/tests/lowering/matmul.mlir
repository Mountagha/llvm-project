// RUN: toyc-ch7 %s -emit=mlir-affine 2>&1 | FileCheck %s

toy.func @main() {
  %0 = toy.constant dense<[[1.0, 5.0], [3.0, 2.0], [4.0, 6.0]]> : tensor<3x2xf64>
  %1 = toy.constant dense<[[0.0, 10.0], [4.0, -1.0]]> : tensor<2x2xf64>
  %2 = toy.matmul %0, %1 : tensor<3x2xf64>, tensor<2x2xf64> -> tensor<3x2xf64>
  toy.print %2 : tensor<3x2xf64>
  toy.return
}

// CHECK-LABEL: func @main()
// CHECK: %[[ZERO:.*]] = arith.constant 0.000000e+00 : f64
// CHECK: %[[RES:.*]] = memref.alloc() : memref<3x2xf64>

// CHECK: affine.for %[[I:.*]] = 0 to 3 {
// CHECK:   affine.for %[[J:.*]] = 0 to 2 {
// CHECK:     affine.store %[[ZERO]], %[[RES]][%[[I]], %[[J]]] : memref<3x2xf64>
// CHECK:   }
// CHECK: }

// CHECK: affine.for %[[I0:.*]] = 0 to 3 {
// CHECK:   affine.for %[[I1:.*]] = 0 to 2 {
// CHECK:     affine.for %[[I2:.*]] = 0 to 2 {
// CHECK:       %[[ACC:.*]] = affine.load %[[RES]][%[[I0]], %[[I1]]] : memref<3x2xf64>
// CHECK:       %[[MUL:.*]] = arith.mulf
// CHECK:       %[[SUM:.*]] = arith.addf %[[ACC]], %[[MUL]] : f64
// CHECK:       affine.store %[[SUM]], %[[RES]][%[[I0]], %[[I1]]] : memref<3x2xf64>
// CHECK:     }
// CHECK:   }
// CHECK: }

// CHECK: toy.print %[[RES]] : memref<3x2xf64>
// CHECK-NOT: toy.matmul
