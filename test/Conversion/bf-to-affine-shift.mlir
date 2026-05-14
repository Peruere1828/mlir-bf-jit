// RUN: bf-opt --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// bf.left  →  arith.addi %ptr, -1
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @left_op
func.func @left_op(%ptr: index) -> index {
  // CHECK: arith.addi %arg0
  // CHECK-SAME: : index
  %0 = bf.left %ptr : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// bf.right  →  arith.addi %ptr, 1
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @right_op
func.func @right_op(%ptr: index) -> index {
  // CHECK: arith.addi
  %0 = bf.right %ptr : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// bf.shift  →  arith.addi %ptr, offset
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @shift_positive
func.func @shift_positive(%ptr: index) -> index {
  // CHECK: arith.addi
  %0 = bf.shift %ptr, 3 : index -> index
  return %0 : index
}

// CHECK-LABEL: func @shift_negative
func.func @shift_negative(%ptr: index) -> index {
  // CHECK: arith.addi
  %0 = bf.shift %ptr, -2 : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// No affine ops  — all index arithmetic is arith.addi, not affine.apply
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_affine_ops
func.func @no_affine_ops(%ptr: index) -> index {
  // CHECK-NOT: affine.
  %0 = bf.left %ptr : index -> index
  return %0 : index
}
