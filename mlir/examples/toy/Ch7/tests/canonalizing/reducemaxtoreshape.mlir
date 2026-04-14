// RUN: toyc-ch7 -x=mlir %s -emit=mlir -opt 2>&1 | FileCheck %s

module {
	toy.func @main(%arg0: tensor<2x1xf64>) {
		%1 = toy.reduce_max %arg0 {axis = 1 : i64} : tensor<2x1xf64> -> tensor<2xf64>
		toy.print %1 : tensor<2xf64>
		toy.return
	}
}

// CHECK-LABEL: toy.func @main
// CHECK-SAME: (%[[ARG0:.*]]: tensor<2x1xf64>)
// CHECK: %[[RESHAPED:.*]] = toy.reshape{{.*}}%[[ARG0]]{{.*}}to tensor<2xf64>
// CHECK-NOT: toy.reduce_max
// CHECK: toy.print %[[RESHAPED]] : tensor<2xf64>
