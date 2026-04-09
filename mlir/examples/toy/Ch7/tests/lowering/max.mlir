// RUN: toyc-ch7 %s -emit=mlir-affine 2>&1 | FileCheck %s

toy.func @main() {
	%0 = toy.constant dense<[[1.000000e+00, 5.000000e+00],
	                         [3.000000e+00, 2.000000e+00]]> : tensor<2x2xf64>
	%1 = toy.constant dense<[[0.000000e+00, 1.000000e+01],
	                         [4.000000e+00, -1.000000e+00]]> : tensor<2x2xf64>
	%2 = "toy.max"(%0, %1) : (tensor<2x2xf64>, tensor<2x2xf64>) -> tensor<2x2xf64>

	%3 = toy.constant dense<[[1.000000e+00, 5.000000e+00, -2.000000e+00],
	                         [3.000000e+00, 2.000000e+00, 8.000000e+00]]> : tensor<2x3xf64>
	%4 = toy.constant dense<[4.000000e+00, 1.000000e+01, 6.000000e+00]> : tensor<3xf64>
	%5 = "toy.max"(%3, %4) : (tensor<2x3xf64>, tensor<3xf64>) -> tensor<2x3xf64>

	%6 = toy.constant dense<[[1.000000e+00, 5.000000e+00, -2.000000e+00],
	                         [3.000000e+00, 2.000000e+00, 8.000000e+00]]> : tensor<2x3xf64>
	%7 = toy.constant dense<[[4.000000e+00, 1.000000e+01, 6.000000e+00]]> : tensor<1x3xf64>
	%8 = "toy.max"(%6, %7) : (tensor<2x3xf64>, tensor<1x3xf64>) -> tensor<2x3xf64>

	%9 = toy.constant dense<[[4.000000e+00],
	                         [7.000000e+00]]> : tensor<2x1xf64>
	%10 = toy.constant dense<[[1.000000e+00, 5.000000e+00, -2.000000e+00],
	                          [3.000000e+00, 2.000000e+00, 8.000000e+00]]> : tensor<2x3xf64>
	%11 = "toy.max"(%9, %10) : (tensor<2x1xf64>, tensor<2x3xf64>) -> tensor<2x3xf64>

	toy.print %2 : tensor<2x2xf64>
	toy.print %5 : tensor<2x3xf64>
	toy.print %8 : tensor<2x3xf64>
	toy.print %11 : tensor<2x3xf64>
	toy.return
}

// CHECK-LABEL: func @main()
// CHECK: affine.for %[[I0:.*]] = 0 to 2 {
// CHECK:   affine.for %[[J0:.*]] = 0 to 2 {
// CHECK:     %[[L0:.*]] = affine.load %{{.*}}[%[[I0]], %[[J0]]] : memref<2x2xf64>
// CHECK:     %[[R0:.*]] = affine.load %{{.*}}[%[[I0]], %[[J0]]] : memref<2x2xf64>
// CHECK:     %[[M0:.*]] = arith.maximumf %[[L0]], %[[R0]] : f64
// CHECK:     affine.store %[[M0]], %{{.*}}[%[[I0]], %[[J0]]] : memref<2x2xf64>

// CHECK: affine.for %[[I1:.*]] = 0 to 2 {
// CHECK:   affine.for %[[J1:.*]] = 0 to 3 {
// CHECK:     %[[L1:.*]] = affine.load %{{.*}}[%[[I1]], %[[J1]]] : memref<2x3xf64>
// CHECK:     %[[R1:.*]] = affine.load %{{.*}}[%[[J1]]] : memref<3xf64>
// CHECK:     %[[M1:.*]] = arith.maximumf %[[L1]], %[[R1]] : f64
// CHECK:     affine.store %[[M1]], %{{.*}}[%[[I1]], %[[J1]]] : memref<2x3xf64>

// CHECK: affine.for %[[I2:.*]] = 0 to 2 {
// CHECK:   affine.for %[[J2:.*]] = 0 to 3 {
// CHECK:     %[[L2:.*]] = affine.load %{{.*}}[%[[I2]], %[[J2]]] : memref<2x3xf64>
// CHECK:     %[[R2:.*]] = affine.load %{{.*}}[0, %[[J2]]] : memref<1x3xf64>
// CHECK:     %[[M2:.*]] = arith.maximumf %[[L2]], %[[R2]] : f64
// CHECK:     affine.store %[[M2]], %{{.*}}[%[[I2]], %[[J2]]] : memref<2x3xf64>

// CHECK: affine.for %[[I3:.*]] = 0 to 2 {
// CHECK:   affine.for %[[J3:.*]] = 0 to 3 {
// CHECK:     %[[L3:.*]] = affine.load %{{.*}}[%[[I3]], 0] : memref<2x1xf64>
// CHECK:     %[[R3:.*]] = affine.load %{{.*}}[%[[I3]], %[[J3]]] : memref<2x3xf64>
// CHECK:     %[[M3:.*]] = arith.maximumf %[[L3]], %[[R3]] : f64
// CHECK:     affine.store %[[M3]], %{{.*}}[%[[I3]], %[[J3]]] : memref<2x3xf64>

// CHECK: toy.print %{{.*}} : memref<2x2xf64>
// CHECK: toy.print %{{.*}} : memref<2x3xf64>
// CHECK: toy.print %{{.*}} : memref<2x3xf64>
// CHECK: toy.print %{{.*}} : memref<2x3xf64>
// CHECK-NOT: toy.max
