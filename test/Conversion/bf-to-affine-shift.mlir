// RUN: bf-opt --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// bf.left  →  arith.addi %ptr, -1
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @left_op
func.func @left_op(%ptr: index) -> index {
  // CHECK: affine.apply
  %0 = bf.left %ptr : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// bf.right  →  affine.apply
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @right_op
func.func @right_op(%ptr: index) -> index {
  // CHECK: affine.apply
  %0 = bf.right %ptr : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// bf.shift  →  affine.apply
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @shift_positive
func.func @shift_positive(%ptr: index) -> index {
  // CHECK: affine.apply
  %0 = bf.shift %ptr, 3 : index -> index
  return %0 : index
}

// CHECK-LABEL: func @shift_negative
func.func @shift_negative(%ptr: index) -> index {
  // CHECK: affine.apply
  %0 = bf.shift %ptr, -2 : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Function-level shifts produce affine.apply
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @affine_shifts
func.func @affine_shifts(%ptr: index) -> index {
  // CHECK: affine.apply
  %0 = bf.left %ptr : index -> index
  return %0 : index
}
