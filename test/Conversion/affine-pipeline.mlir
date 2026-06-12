// RUN: bf-opt --bf-lower-to-affine-for --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// Full pipeline: bf.loop → scf.for → memref ops
// [->+<] pattern: decrement ptr[0], increment ptr[1]
//===----------------------------------------------------------------------===//

// Module-level: tape global is created ONCE before all functions
// CHECK: memref.global "private" @bf_tape : memref<30000xi8>

// CHECK-LABEL: func @full_pipeline_move
func.func @full_pipeline_move(%ptr: index) -> index {
  // CHECK:      memref.load {{.*}}[%arg0] : memref<30000xi8>
  // CHECK:      arith.index_castui
  // CHECK:      memref.store {{.*}}, {{.*}}[%arg0] : memref<30000xi8>
  // CHECK:      scf.for %{{.*}} = %{{.*}} to %{{.*}} step %{{.*}} iter_args(%{{.*}} = %arg0) -> (index) {
  // CHECK:        arith.addi
  // CHECK:        memref.load
  // CHECK:        arith.addi
  // CHECK:        memref.store
  // CHECK:        scf.yield
  // CHECK:      }
  // CHECK-NOT:  bf.
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.right %ptr : index -> index
    bf.modify %1, 1 : index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Full pipeline: simple decrement ([-] pattern)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @full_pipeline_simple
func.func @full_pipeline_simple(%ptr: index) -> index {
  // CHECK:      memref.load
  // CHECK:      arith.index_castui
  // CHECK:      scf.for
  // CHECK:        scf.yield
  // CHECK-NOT:  bf.
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Non-loop code: function-level ops become affine ops
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_loop_affine
func.func @no_loop_affine(%ptr: index) -> index {
  // CHECK:      affine.apply
  // CHECK:      affine.load
  // CHECK:      affine.store
  // CHECK-NOT:  bf.
  %0 = bf.shift %ptr, 3 : index -> index
  bf.modify %0, 5 : index
  return %0 : index
}
