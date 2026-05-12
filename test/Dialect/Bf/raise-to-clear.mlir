// RUN: bf-opt --bf-raise-to-clear %s | FileCheck %s

//===----------------------------------------------------------------------===//
// Raise loop { modify +1 }  →  bf.clear
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @raise_modify_plus_one
func.func @raise_modify_plus_one(%arg0: index) -> index {
  // CHECK: bf.clear %arg0
  // CHECK-NOT: bf.loop
  %0 = bf.loop(%arg0) {
    bf.modify %arg0, 1 : index
    bf.yield %arg0 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Raise loop { modify -1 }  →  bf.clear
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @raise_modify_minus_one
func.func @raise_modify_minus_one(%arg0: index) -> index {
  // CHECK: bf.clear %arg0
  // CHECK-NOT: bf.loop
  %0 = bf.loop(%arg0) {
    bf.modify %arg0, -1 : index
    bf.yield %arg0 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Raise loop { bf.clear }  →  bf.clear (remove nesting)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @raise_nested_clear
func.func @raise_nested_clear(%arg0: index) -> index {
  // CHECK: bf.clear %arg0
  // CHECK-NOT: bf.loop
  %0 = bf.loop(%arg0) {
    bf.clear %arg0 : index
    bf.yield %arg0 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative tests — should NOT be converted
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_raise_modify_two
func.func @no_raise_modify_two(%arg0: index) -> index {
  // delta != ±1  →  loop stays
  // CHECK: bf.loop
  %0 = bf.loop(%arg0) {
    bf.modify %arg0, 2 : index
    bf.yield %arg0 : index
  } : index -> index
  return %0 : index
}

// CHECK-LABEL: func @no_raise_non_modify
func.func @no_raise_non_modify(%arg0: index) -> index {
  // body op is not modify or clear  →  loop stays
  // CHECK: bf.loop
  %0 = bf.loop(%arg0) {
    bf.add %arg0 : index
    bf.yield %arg0 : index
  } : index -> index
  return %0 : index
}

// CHECK-LABEL: func @no_raise_multiple_body_ops
func.func @no_raise_multiple_body_ops(%arg0: index) -> index {
  // more than one body op  →  loop stays
  // CHECK: bf.loop
  %0 = bf.loop(%arg0) {
    bf.modify %arg0, -1 : index
    %1 = bf.shift %arg0, 1 : index -> index
    bf.yield %1 : index
  } : index -> index
  return %0 : index
}

// CHECK-LABEL: func @no_raise_empty_loop
func.func @no_raise_empty_loop(%arg0: index) -> index {
  // only yield, no body op  →  loop stays
  // CHECK: bf.loop
  %0 = bf.loop(%arg0) {
    bf.yield %arg0 : index
  } : index -> index
  return %0 : index
}
