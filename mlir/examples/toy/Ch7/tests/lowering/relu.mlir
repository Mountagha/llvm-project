// RUN: toyc-ch7 %s -emit=mlir-affine 2>&1 | FileCheck %s

toy.func @main() {
	%0 = toy.constant dense<[[1.000000e+00, -5.000000e+00],
													 [3.000000e+00, -2.000000e+00]]> : tensor<2x2xf64>
	%1 = toy.relu %0 : tensor<2x2xf64>
	toy.print %1 : tensor<2x2xf64>
	toy.return
}

// CHECK-LABEL: func @main()
// CHECK: %[[ZERO:.*]] = arith.constant 0.000000e+00 : f64
// CHECK: affine.for %[[I:.*]] = 0 to 2 {
// CHECK:   affine.for %[[J:.*]] = 0 to 2 {
// CHECK:     %[[IN:.*]] = affine.load %{{.*}}[%[[I]], %[[J]]] : memref<2x2xf64>
// CHECK:     %[[OUT:.*]] = arith.maximumf %[[IN]], %[[ZERO]] : f64
// CHECK:     affine.store %[[OUT]], %{{.*}}[%[[I]], %[[J]]] : memref<2x2xf64>
// CHECK: toy.print %{{.*}} : memref<2x2xf64>
// CHECK-NOT: toy.relu
