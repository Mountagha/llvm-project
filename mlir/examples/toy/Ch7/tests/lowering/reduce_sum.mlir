// the add so that we can avoid the folding to be called before the lowering.
// RUN: toyc-ch7 %s -emit=mlir-affine 2>&1 | FileCheck %s

toy.func @main() {
  %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00],
                           [3.000000e+00, 4.000000e+00]]> : tensor<2x2xf64>
  %1 = toy.add %0, %0 : tensor<2x2xf64>
  %2 = toy.reduce_sum %1 {axis = 1 : i64} : tensor<2x2xf64> -> tensor<2xf64>
  toy.print %2 : tensor<2xf64>
  toy.return
}

// CHECK-LABEL: func @main()
// CHECK: %[[ZERO:.*]] = arith.constant 0.000000e+00 : f64
// CHECK: %[[RED:.*]] = memref.alloc() : memref<2xf64>
// CHECK: %[[TMP:.*]] = memref.alloc() : memref<2x2xf64>
// CHECK: %[[IN:.*]] = memref.alloc() : memref<2x2xf64>

// CHECK: affine.for %[[I:.*]] = 0 to 2 {
// CHECK:   affine.store %[[ZERO]], %[[RED]][%[[I]]] : memref<2xf64>
// CHECK: }


// CHECK: affine.for %[[I0:.*]] = 0 to 2 {
// CHECK:   affine.for %[[I1:.*]] = 0 to 2 {
// CHECK:     %[[ELEM:.*]] = affine.load %[[TMP]][%[[I0]], %[[I1]]] : memref<2x2xf64>
// CHECK:     %[[ACC:.*]] = affine.load %[[RED]][%[[I0]]] : memref<2xf64>
// CHECK:     %[[SUM:.*]] = arith.addf %[[ACC]], %[[ELEM]] : f64
// CHECK:     affine.store %[[SUM]], %[[RED]][%[[I0]]] : memref<2xf64>

// CHECK: toy.print %[[RED]] : memref<2xf64>
// CHECK-NOT: toy.reduce_sum