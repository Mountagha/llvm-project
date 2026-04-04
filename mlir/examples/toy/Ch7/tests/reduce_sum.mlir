// RUN: split-file %s %t
// RUN: toyc-ch7 -x=mlir -emit=mlir -opt %t/reduction_performed.mlir 2>&1 | FileCheck %s --check-prefix=REDUCE
// RUN: not toyc-ch7 -x=mlir -emit=mlir %t/rank_zero.mlir 2>&1 | FileCheck %s --check-prefix=RANK-ZERO
// RUN: not toyc-ch7 -x=mlir -emit=mlir %t/axis_oob.mlir 2>&1 | FileCheck %s --check-prefix=AXIS-OOB
// RUN: not toyc-ch7 -x=mlir -emit=mlir %t/axis_negative.mlir 2>&1 | FileCheck %s --check-prefix=AXIS-NEGATIVE
// RUN: not toyc-ch7 -x=mlir -emit=mlir %t/result_shape.mlir 2>&1 | FileCheck %s --check-prefix=RESULT-SHAPE

// ReduceSumOp verifier robustness tests.

//--- reduction_performed.mlir

module {
	toy.func @reduction_performed() {
		%0 = toy.constant dense<[[1.000000e+00, 5.000000e+00],
														 [3.000000e+00, 2.000000e+00]]> : tensor<2x2xf64>
		%1 = toy.reduce_sum %0 {axis = 1 : i64} : tensor<2x2xf64> -> tensor<2xf64>
		toy.print %1 : tensor<2xf64>
		toy.return
	}
}

// REDUCE-LABEL: toy.func @reduction_performed()
// REDUCE: [[CST:%.*]] = toy.constant dense<[6.000000e+00, 5.000000e+00]> : tensor<2xf64>
// REDUCE-NOT: toy.reduce_sum
// REDUCE: toy.print [[CST]] : tensor<2xf64>
// REDUCE: toy.return

//--- rank_zero.mlir

module {
	toy.func @rank_zero() {
		%0 = toy.constant dense<1.000000e+00> : tensor<f64>
		%1 = toy.reduce_sum %0 {axis = 0 : i64} : tensor<f64> -> tensor<f64>
		toy.print %1 : tensor<f64>
		toy.return
	}
}

// RANK-ZERO: error: 'toy.reduce_sum' op requires input tensor with rank > 0

//--- axis_oob.mlir

module {
	toy.func @axis_oob() {
		%0 = toy.constant dense<[[1.000000e+00, 5.000000e+00], [3.000000e+00, 2.000000e+00]]> : tensor<2x2xf64>
		%1 = toy.reduce_sum %0 {axis = 2 : i64} : tensor<2x2xf64> -> tensor<2xf64>
		toy.print %1 : tensor<2xf64>
		toy.return
	}
}

// AXIS-OOB: error: 'toy.reduce_sum' op axis 2{{.*}}out of bounds for tensor of rank{{.*}}2

//--- axis_negative.mlir

module {
	toy.func @axis_negative() {
		%0 = toy.constant dense<[[1.000000e+00, 5.000000e+00], [3.000000e+00, 2.000000e+00]]> : tensor<2x2xf64>
		%1 = toy.reduce_sum %0 {axis = -1 : i64} : tensor<2x2xf64> -> tensor<2xf64>
		toy.print %1 : tensor<2xf64>
		toy.return
	}
}

// AXIS-NEGATIVE: error: 'toy.reduce_sum' op axis -1{{.*}}out of bounds for tensor of rank{{.*}}2

//--- result_shape.mlir

module {
	toy.func @result_shape_mismatch() {
		%0 = toy.constant dense<[[1.000000e+00, 5.000000e+00], [3.000000e+00, 2.000000e+00]]> : tensor<2x2xf64>
		%1 = toy.reduce_sum %0 {axis = 1 : i64} : tensor<2x2xf64> -> tensor<3xf64>
		toy.print %1 : tensor<3xf64>
		toy.return
	}
}

// RESULT-SHAPE: error: 'toy.reduce_sum' op has incorrect result type: expected tensor<2xf64> but got tensor<3xf64>
