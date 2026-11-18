
 // RUN: toyc-ch7 -emit=toy-linalg-tile %s | FileCheck %s

 // CHECK-LABEL: func.func @map_1d(
 // CHECK: scf.for
 // CHECK: affine.min
 // CHECK: tensor.extract_slice
 // CHECK-SAME: tensor<100xf64> to tensor<?xf64>
 // CHECK: tensor.extract_slice
 // CHECK-SAME: tensor<100xf64> to tensor<?xf64>
 // CHECK: linalg.generic
 // CHECK: iterator_types = ["parallel"]
 // CHECK: tensor.insert_slice
 // CHECK: scf.yield
 // CHECK: return

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
