// RUN: bf-opt --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// bf.add  →  memref.load + arith.addi 1 + memref.store
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @add_op
func.func @add_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK: memref.load
  // CHECK: arith.addi
  // CHECK: memref.store
  bf.add %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.sub  →  memref.load + arith.addi -1 + memref.store
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @sub_op
func.func @sub_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK: memref.load
  // CHECK: arith.addi
  // CHECK: memref.store
  bf.sub %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.clear  →  memref.store(c0_i8)  (no load needed)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @clear_op
func.func @clear_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK-NOT: memref.load
  // CHECK: arith.constant 0
  // CHECK: memref.store
  bf.clear %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.modify  →  memref.load + arith.addi delta + memref.store
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @modify_positive
func.func @modify_positive(%ptr: index) {
  // CHECK: memref.load
  // CHECK: arith.addi
  // CHECK: memref.store
  bf.modify %ptr, 5 : index
  return
}

// CHECK-LABEL: func @modify_negative
func.func @modify_negative(%ptr: index) {
  bf.modify %ptr, -3 : index
  return
}

//===----------------------------------------------------------------------===//
// No bf ops remain after conversion
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_bf_ops_remain
func.func @no_bf_ops_remain(%ptr: index) {
  // CHECK-NOT: bf.
  bf.add %ptr : index
  bf.sub %ptr : index
  bf.clear %ptr : index
  return
}
