// RUN: bf-opt --canonicalize %s | FileCheck %s

//===----------------------------------------------------------------------===//
// ShiftOp fold: offset=0  →  folded to the input pointer
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @remove_zero_shift
func.func @remove_zero_shift(%arg0: index) -> index {
  // CHECK: return %arg0
  %0 = bf.shift %arg0, 0 : index -> index
  return %0 : index
}

// CHECK-LABEL: func @keep_nonzero_shift
func.func @keep_nonzero_shift(%arg0: index) -> index {
  // CHECK: bf.shift
  // CHECK-SAME: 5
  %0 = bf.shift %arg0, 5 : index -> index
  return %0 : index
}

// CHECK-LABEL: func @remove_zero_shift_chain
func.func @remove_zero_shift_chain(%arg0: index) -> index {
  // CHECK: return %arg0
  %0 = bf.shift %arg0, 0 : index -> index
  %1 = bf.shift %0, 0 : index -> index
  return %1 : index
}

//===----------------------------------------------------------------------===//
// ModifyOp canonicalize: delta=0  →  erased
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @remove_zero_modify
func.func @remove_zero_modify(%arg0: index) {
  // CHECK-NOT: bf.modify
  bf.modify %arg0, 0 : index
  return
}

// CHECK-LABEL: func @keep_nonzero_modify
func.func @keep_nonzero_modify(%arg0: index) {
  // CHECK: bf.modify
  // CHECK-SAME: -3
  bf.modify %arg0, -3 : index
  return
}

// CHECK-LABEL: func @skip_zero_deltas
func.func @skip_zero_deltas(%arg0: index) {
  // CHECK: bf.modify
  // CHECK-SAME: 3
  // CHECK-NOT: bf.modify
  bf.modify %arg0, 3 : index
  bf.modify %arg0, 0 : index
  bf.modify %arg0, 0 : index
  return
}

//===----------------------------------------------------------------------===//
// ClearOp canonicalize: redundant consecutive clears on same ptr  →  removed
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @remove_redundant_clear
func.func @remove_redundant_clear(%arg0: index) {
  // CHECK: bf.clear %arg0
  // CHECK-NOT: bf.clear
  bf.clear %arg0 : index
  bf.clear %arg0 : index
  return
}

// CHECK-LABEL: func @keep_clears_on_different_ptrs
func.func @keep_clears_on_different_ptrs(%arg0: index, %arg1: index) {
  // CHECK-COUNT-2: bf.clear
  bf.clear %arg0 : index
  bf.clear %arg1 : index
  return
}

// CHECK-LABEL: func @clear_then_modify_both_kept
func.func @clear_then_modify_both_kept(%arg0: index) {
  // CHECK: bf.clear
  // CHECK: bf.modify
  bf.clear %arg0 : index
  bf.modify %arg0, 1 : index
  return
}

// CHECK-LABEL: func @three_clears_keep_first
func.func @three_clears_keep_first(%arg0: index) {
  // CHECK: bf.clear %arg0
  // CHECK-NOT: bf.clear
  bf.clear %arg0 : index
  bf.clear %arg0 : index
  bf.clear %arg0 : index
  return
}
