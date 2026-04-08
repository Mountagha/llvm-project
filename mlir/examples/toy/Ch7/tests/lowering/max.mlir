// RUN: toyc-ch7 %s -emit=mlir-affine 2>&1 | FileCheck %s

toy.func @main() {
	%0 = toy.constant dense<[[1.000000e+00, 5.000000e+00],
													 [3.000000e+00, 2.000000e+00]]> : tensor<2x2xf64>
	%1 = toy.constant dense<[[0.000000e+00, 1.000000e+01],
													 [4.000000e+00, -1.000000e+00]]> : tensor<2x2xf64>
	//%2 = toy.max %0, %1 : tensor<2x2xf64>
    %2 = "toy.max"(%0, %1) : (tensor<2x2xf64>, tensor<2x2xf64>) -> tensor<2x2xf64>
	toy.print %2 : tensor<2x2xf64>
	toy.return
}

// CHECK-LABEL: func @main()
// CHECK: %[[RES:.*]] = memref.alloc() : memref<2x2xf64>
// CHECK: %[[RHS:.*]] = memref.alloc() : memref<2x2xf64>
// CHECK: %[[LHS:.*]] = memref.alloc() : memref<2x2xf64>
// CHECK: affine.for %[[I:.*]] = 0 to 2 {
// CHECK:   affine.for %[[J:.*]] = 0 to 2 {
// CHECK:     %[[L:.*]] = affine.load %[[LHS]][%[[I]], %[[J]]] : memref<2x2xf64>
// CHECK:     %[[R:.*]] = affine.load %[[RHS]][%[[I]], %[[J]]] : memref<2x2xf64>
// CHECK:     %[[M:.*]] = arith.maximumf %[[L]], %[[R]] : f64
// CHECK:     affine.store %[[M]], %[[RES]][%[[I]], %[[J]]] : memref<2x2xf64>
// CHECK: toy.print %[[RES]] : memref<2x2xf64>
// CHECK-NOT: toy.max
