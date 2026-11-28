

// RUN: toyc-ch7 -x=mlir -emit=toy-linalg-vectorize %s 2>&1 | FileCheck %s --implicit-check-not=linalg.generic --implicit-check-not=affine.min

// CHECK-LABEL: func.func @map_1d(

// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<32xf64>
// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<4xf64>
// CHECK-DAG: %[[C96:[a-zA-Z0-9_]+]] = arith.constant 96 : index
// CHECK-DAG: %[[C32:[a-zA-Z0-9_]+]] = arith.constant 32 : index
// CHECK-DAG: %[[C0:[a-zA-Z0-9_]+]] = arith.constant 0 : index

// Main loop.
// CHECK: scf.for %[[IV:[a-zA-Z0-9_]+]] = %[[C0]] to %[[C96]] step %[[C32]]

// CHECK: tensor.extract_slice
// CHECK-SAME: [%[[IV]]] [32] [1]
// CHECK-SAME: to tensor<32xf64>

// CHECK: %[[READ:.*]] = vector.transfer_read
// CHECK-SAME: vector<32xf64>

// CHECK: %[[MUL:.*]] = arith.mulf %[[READ]],
// CHECK-SAME: vector<32xf64>

// CHECK: %[[WRITE:.*]] = vector.transfer_write %[[MUL]]
// CHECK-SAME: tensor<32xf64>

// CHECK: %[[INSERT:.*]] = tensor.insert_slice %[[WRITE]]
// CHECK: scf.yield %[[INSERT]]

// Remainder.
// CHECK: tensor.extract_slice
// CHECK-SAME: [96] [4] [1]
// CHECK-SAME: to tensor<4xf64>

// CHECK: %[[TAIL_READ:.*]] = vector.transfer_read
// CHECK-SAME: vector<4xf64>

// CHECK: %[[TAIL_MUL:.*]] = arith.mulf %[[TAIL_READ]],
// CHECK-SAME: vector<4xf64>

// CHECK: %[[TAIL_WRITE:.*]] = vector.transfer_write %[[TAIL_MUL]]
// CHECK-SAME: tensor<4xf64>

// CHECK: %[[RESULT:.*]] = tensor.insert_slice %[[TAIL_WRITE]]
// CHECK-SAME: [96] [4] [1]

// CHECK: return %[[RESULT]]


 module {
   func.func @map_1d(
       %input: tensor<100xf64>
   ) -> tensor<100xf64> {
     %empty = tensor.empty() : tensor<100xf64>
     %result = linalg.generic {
       indexing_maps = [
         affine_map<(d0) -> (d0)>,
         affine_map<(d0) -> (d0)>
       ],
       iterator_types = ["parallel"]
     }
     ins(%input : tensor<100xf64>)
     outs(%empty : tensor<100xf64>) {
     ^bb0(%in: f64, %out: f64):
       %two = arith.constant 2.0 : f64
       %value = arith.mulf %in, %two : f64
       linalg.yield %value : f64
     } -> tensor<100xf64>

     return %result : tensor<100xf64>
   }
 }
