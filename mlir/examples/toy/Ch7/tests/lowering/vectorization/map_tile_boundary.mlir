
// RUN: toyc-ch7 -x=mlir -emit=toy-linalg-vectorize %s 2>&1 | FileCheck %s --implicit-check-not=linalg.generic --implicit-check-not=affine.min

// CHECK-LABEL: func.func @map_boundary(

// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<32x32xf64>
// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<32x18xf64>
// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<6x32xf64>
// CHECK-DAG: arith.constant dense<2.000000e+00> : vector<6x18xf64>

// Main tile.
// CHECK: vector.transfer_read
// CHECK-SAME: vector<32x32xf64>
// CHECK: arith.mulf
// CHECK-SAME: vector<32x32xf64>
// CHECK: vector.transfer_write
// CHECK-SAME: vector<32x32xf64>

// Right boundary.
// CHECK: vector.transfer_read
// CHECK-SAME: vector<32x18xf64>
// CHECK: arith.mulf
// CHECK-SAME: vector<32x18xf64>
// CHECK: vector.transfer_write
// CHECK-SAME: vector<32x18xf64>

// Bottom boundary.
// CHECK: vector.transfer_read
// CHECK-SAME: vector<6x32xf64>
// CHECK: arith.mulf
// CHECK-SAME: vector<6x32xf64>
// CHECK: vector.transfer_write
// CHECK-SAME: vector<6x32xf64>

// Bottom-right corner.
// CHECK: vector.transfer_read
// CHECK-SAME: vector<6x18xf64>
// CHECK: arith.mulf
// CHECK-SAME: vector<6x18xf64>
// CHECK: vector.transfer_write
// CHECK-SAME: vector<6x18xf64>

// CHECK: return



 module {
   func.func @map_boundary(
       %input: tensor<70x50xf64>
   ) -> tensor<70x50xf64> {
     %empty = tensor.empty() : tensor<70x50xf64>
     %result = linalg.generic {
       indexing_maps = [
         affine_map<(d0, d1) -> (d0, d1)>,
         affine_map<(d0, d1) -> (d0, d1)>
       ],
       iterator_types = ["parallel", "parallel"]
     }
     ins(%input : tensor<70x50xf64>)
     outs(%empty : tensor<70x50xf64>) {
     ^bb0(%in: f64, %out: f64):
       %two = arith.constant 2.0 : f64
       %value = arith.mulf %in, %two : f64
       linalg.yield %value : f64
     } -> tensor<70x50xf64>

     return %result : tensor<70x50xf64>
   }
 }