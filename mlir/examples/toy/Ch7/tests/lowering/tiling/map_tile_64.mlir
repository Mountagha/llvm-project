
 // RUN: toyc-ch7 -emit=toy-linalg-tile %s | FileCheck %s

 // CHECK-LABEL: func.func @map_64(
 // CHECK: scf.for
 // CHECK-SAME: step %[[STEP:.*]]
 // CHECK: scf.for
 // CHECK-SAME: step %[[STEP]]
 // CHECK: tensor.extract_slice
 // CHECK: tensor.extract_slice
 // CHECK: linalg.generic
 // CHECK: iterator_types = ["parallel", "parallel"]
 // CHECK: tensor.insert_slice
 // CHECK: scf.yield
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
