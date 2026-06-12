// RUN: bf-opt --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// bf.add  →  affine.load + arith.addi 1 + affine.store (function-level)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @add_op
func.func @add_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK: affine.load
  // CHECK: arith.addi
  // CHECK: affine.store
  bf.add %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.sub  →  affine.load + arith.addi -1 + affine.store
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @sub_op
func.func @sub_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK: affine.load
  // CHECK: arith.addi
  // CHECK: affine.store
  bf.sub %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.clear  →  affine.store(c0_i8)  (no load needed)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @clear_op
func.func @clear_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK-NOT: affine.load
  // CHECK: arith.constant 0
  // CHECK: affine.store
  bf.clear %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.modify  →  affine.load + arith.addi delta + affine.store
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @modify_positive
func.func @modify_positive(%ptr: index) {
  // CHECK: affine.load
  // CHECK: arith.addi
  // CHECK: affine.store
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
