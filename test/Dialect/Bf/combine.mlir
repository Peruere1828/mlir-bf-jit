// RUN: bf-opt --bf-combine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// Lift: bf.left  →  bf.shift -1
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @lift_left
func.func @lift_left(%arg0: index) -> index {
  // CHECK: %[[R:.*]] = bf.shift %arg0, -1
  // CHECK: return %[[R]]
  %0 = bf.left %arg0 : index -> index
  return %0 : index
}

// CHECK-LABEL: func @lift_right
func.func @lift_right(%arg0: index) -> index {
  // CHECK: %[[R:.*]] = bf.shift %arg0, 1
  // CHECK: return %[[R]]
  %0 = bf.right %arg0 : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Lift: bf.add  →  bf.modify 1   /   bf.sub  →  bf.modify -1
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @lift_add
func.func @lift_add(%arg0: index) {
  // CHECK: bf.modify %arg0, 1
  bf.add %arg0 : index
  return
}

// CHECK-LABEL: func @lift_sub
func.func @lift_sub(%arg0: index) {
  // CHECK: bf.modify %arg0, -1
  bf.sub %arg0 : index
  return
}

//===----------------------------------------------------------------------===//
// Merge consecutive shifts (chained via SSA)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @merge_shifts
func.func @merge_shifts(%arg0: index) -> index {
  // CHECK: %[[R:.*]] = bf.shift %arg0, 2
  // CHECK: return %[[R]]
  // CHECK-NOT: bf.shift
  %0 = bf.shift %arg0, 1 : index -> index
  %1 = bf.shift %0, 2 : index -> index
  %2 = bf.shift %1, -1 : index -> index
  return %2 : index
}

// CHECK-LABEL: func @merge_shifts_cancel
func.func @merge_shifts_cancel(%arg0: index) -> index {
  // shift 3 + shift -3 = shift 0, then folder erases shift 0 entirely
  // CHECK: return %arg0
  // CHECK-NOT: bf.shift
  %0 = bf.shift %arg0, 3 : index -> index
  %1 = bf.shift %0, -3 : index -> index
  return %1 : index
}

// CHECK-LABEL: func @no_merge_shifts_broken_chain
func.func @no_merge_shifts_broken_chain(%arg0: index, %arg1: index) -> index {
  // Two shifts using different pointers — should NOT merge
  // CHECK-COUNT-2: bf.shift
  %0 = bf.shift %arg0, 1 : index -> index
  %1 = bf.shift %arg1, 2 : index -> index
  return %1 : index
}

// CHECK-LABEL: func @no_merge_shifts_intervening_op
func.func @no_merge_shifts_intervening_op(%arg0: index) -> index {
  // A non-shift op between shifts breaks the merge
  // CHECK-COUNT-2: bf.shift
  %0 = bf.shift %arg0, 1 : index -> index
  bf.modify %0, 1 : index
  %1 = bf.shift %0, 2 : index -> index
  return %1 : index
}

//===----------------------------------------------------------------------===//
// Merge consecutive modifies (same pointer operand)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @merge_modifies
func.func @merge_modifies(%arg0: index) {
  // CHECK: bf.modify %arg0, 3
  // CHECK-NOT: bf.modify
  bf.modify %arg0, 1 : index
  bf.modify %arg0, 2 : index
  bf.modify %arg0, 0 : index
  return
}

// CHECK-LABEL: func @merge_modifies_cancel
func.func @merge_modifies_cancel(%arg0: index) {
  // CHECK: bf.modify %arg0, 0
  bf.modify %arg0, 5 : index
  bf.modify %arg0, -5 : index
  return
}

// CHECK-LABEL: func @no_merge_modifies_different_ptr
func.func @no_merge_modifies_different_ptr(%arg0: index, %arg1: index) {
  // CHECK-COUNT-2: bf.modify
  bf.modify %arg0, 1 : index
  bf.modify %arg1, 2 : index
  return
}

// CHECK-LABEL: func @no_merge_modifies_intervening_op
func.func @no_merge_modifies_intervening_op(%arg0: index) {
  // CHECK-COUNT-2: bf.modify
  bf.modify %arg0, 1 : index
  %0 = bf.shift %arg0, 1 : index -> index
  bf.modify %arg0, 2 : index
  return
}

//===----------------------------------------------------------------------===//
// Combined: lift + merge (greedy rewriter applies both)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @lift_and_merge_lefts
func.func @lift_and_merge_lefts(%arg0: index) -> index {
  // three lefts  →  shift -3
  // CHECK: %[[R:.*]] = bf.shift %arg0, -3
  // CHECK: return %[[R]]
  %0 = bf.left %arg0 : index -> index
  %1 = bf.left %0 : index -> index
  %2 = bf.left %1 : index -> index
  return %2 : index
}

// CHECK-LABEL: func @lift_and_merge_adds
func.func @lift_and_merge_adds(%arg0: index) {
  // two adds on same ptr  →  modify 2
  // CHECK: bf.modify %arg0, 2
  bf.add %arg0 : index
  bf.add %arg0 : index
  return
}

// CHECK-LABEL: func @lift_and_merge_mixed_pointer_ops
func.func @lift_and_merge_mixed_pointer_ops(%arg0: index) -> index {
  // left + left + right  →  shift -1 (lifted then merged)
  // CHECK: %[[R:.*]] = bf.shift %arg0, -1
  // CHECK: return %[[R]]
  %0 = bf.left %arg0 : index -> index
  %1 = bf.left %0 : index -> index
  %2 = bf.right %1 : index -> index
  return %2 : index
}

// CHECK-LABEL: func @lift_and_merge_mixed_cell_ops
func.func @lift_and_merge_mixed_cell_ops(%arg0: index) {
  // add + add + sub  →  modify 1
  // CHECK: bf.modify %arg0, 1
  bf.add %arg0 : index
  bf.add %arg0 : index
  bf.sub %arg0 : index
  return
}
