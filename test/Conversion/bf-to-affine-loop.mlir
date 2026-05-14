// RUN: bf-opt --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// bf.loop  →  scf.while with before/after regions
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @simple_loop
func.func @simple_loop(%ptr: index) -> index {
  // CHECK: scf.while
  // CHECK:   memref.get_global @bf_tape
  // CHECK:   memref.load
  // CHECK:   arith.cmpi ne
  // CHECK:   scf.condition
  // CHECK: } do {
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Loop with pointer movement inside the body
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @loop_with_shift
func.func @loop_with_shift(%ptr: index) -> index {
  // CHECK: scf.while
  // CHECK:   scf.condition
  // CHECK: } do {
  // CHECK:   arith.addi
  // CHECK:   scf.yield
  // CHECK: }
  %0 = bf.loop(%ptr) {
    %1 = bf.right %ptr : index -> index
    bf.yield %1 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Loop with cell operation AND pointer shift ([->+<] pattern body)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @loop_move_cell
func.func @loop_move_cell(%ptr: index) -> index {
  // CHECK: scf.while
  // CHECK:   scf.condition
  // CHECK: } do {
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   arith.addi
  // CHECK:   scf.yield
  // CHECK: }
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
// No bf ops remain after conversion
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_bf_ops
func.func @no_bf_ops(%ptr: index) -> index {
  // CHECK-NOT: bf.
  %0 = bf.loop(%ptr) {
    bf.clear %ptr : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}
