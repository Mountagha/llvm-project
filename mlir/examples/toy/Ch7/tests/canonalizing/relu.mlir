// RUN: toyc-ch7 -x=mlir %s -emit=mlir -opt 2>&1 | FileCheck %s

module {
	toy.func @main() {
		%0 = toy.constant dense<[[1.000000e+00, -5.000000e+00],
														 [3.000000e+00, -2.000000e+00]]> : tensor<2x2xf64>
		%1 = toy.relu %0 : tensor<2x2xf64>
		%2 = toy.relu %1 : tensor<2x2xf64>
		toy.print %2 : tensor<2x2xf64>
		toy.return
	}
}

// CHECK-LABEL: toy.func @main
// CHECK: %[[INPUT:.*]] = toy.constant dense<{{.*}}> : tensor<2x2xf64>
// CHECK: %[[RELU:.*]] = toy.relu %[[INPUT]] : tensor<2x2xf64>
// CHECK-NOT: toy.relu %[[RELU]]
// CHECK: toy.print %[[RELU]] : tensor<2x2xf64>
