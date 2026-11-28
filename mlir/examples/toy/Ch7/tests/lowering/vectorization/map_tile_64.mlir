
// RUN: toyc-ch7 -x=mlir -emit=toy-linalg-vectorize %s 2>&1 | FileCheck %s --implicit-check-not=linalg.generic --implicit-check-not=affine.min

// CHECK-LABEL: func.func @map_64(

// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<32x32xf64>
// CHECK-DAG: %[[C32:[a-zA-Z0-9_]+]] = arith.constant 32 : index
// CHECK-DAG: %[[C64:[a-zA-Z0-9_]+]] = arith.constant 64 : index
// CHECK-DAG: %[[C0:[a-zA-Z0-9_]+]] = arith.constant 0 : index

// CHECK: scf.for %[[I:[a-zA-Z0-9_]+]] = %[[C0]] to %[[C64]] step %[[C32]]
// CHECK: scf.for %[[J:[a-zA-Z0-9_]+]] = %[[C0]] to %[[C64]] step %[[C32]]

// CHECK: tensor.extract_slice
// CHECK-SAME: [%[[I]], %[[J]]] [32, 32] [1, 1]
// CHECK-SAME: to tensor<32x32xf64>

// CHECK: %[[READ:.*]] = vector.transfer_read
// CHECK-SAME: in_bounds = [true, true]
// CHECK-SAME: vector<32x32xf64>

// CHECK: %[[MUL:.*]] = arith.mulf %[[READ]],
// CHECK-SAME: vector<32x32xf64>

// CHECK: %[[WRITE:.*]] = vector.transfer_write %[[MUL]]
// CHECK-SAME: tensor<32x32xf64>

// CHECK: %[[INSERT:.*]] = tensor.insert_slice %[[WRITE]]
// CHECK-SAME: [%[[I]], %[[J]]] [32, 32] [1, 1]

// CHECK: scf.yield %[[INSERT]]
// CHECK: scf.yield
// CHECK: return


 module {
   func.func @map_64(
       %input: tensor<64x64xf64>
   ) -> tensor<64x64xf64> {
     %empty = tensor.empty() : tensor<64x64xf64>
     %result = linalg.generic {
       indexing_maps = [
         affine_map<(d0, d1) -> (d0, d1)>,
         affine_map<(d0, d1) -> (d0, d1)>
       ],
       iterator_types = ["parallel", "parallel"]
     }
     ins(%input : tensor<64x64xf64>)
     outs(%empty : tensor<64x64xf64>) {
     ^bb0(%in: f64, %out: f64):
       %two = arith.constant 2.0 : f64
       %value = arith.mulf %in, %two : f64
       linalg.yield %value : f64
     } -> tensor<64x64xf64>

     return %result : tensor<64x64xf64>
   }
 }
