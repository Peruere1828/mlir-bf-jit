// RUN: bf-opt --bf-lower-to-affine-for %s | FileCheck %s

//===----------------------------------------------------------------------===//
// Positive: [->+<]  —  classic move pattern
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @move_to_next_cell
func.func @move_to_next_cell(%ptr: index) -> index {
  // CHECK: memref.load {{.*}}[%arg0] : memref<30000xi8>
  // CHECK: arith.index_castui
  // CHECK: memref.store {{.*}}, {{.*}}[%arg0] : memref<30000xi8>
  // CHECK: scf.for %{{.*}} = %{{.*}} to %{{.*}} step %{{.*}} iter_args(%{{.*}} = %arg0) -> (index) {
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
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
// Positive: [->>+<<]  —  move to cell at offset +2
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @move_to_offset_2
func.func @move_to_offset_2(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.shift %ptr, 2 : index -> index
    bf.modify %1, 1 : index
    %2 = bf.shift %1, -2 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: [->++<]  —  multiply by 2
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @double_value
func.func @double_value(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.right %ptr : index -> index
    bf.modify %1, 2 : index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: [->+>+<<]  —  copy to two cells
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @copy_to_two_cells
func.func @copy_to_two_cells(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.shift %ptr, 1 : index -> index
    bf.modify %1, 1 : index
    %2 = bf.shift %1, 1 : index -> index
    bf.modify %2, 1 : index
    %3 = bf.shift %2, -2 : index -> index
    bf.yield %3 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: [->+>+++<<]  —  move with two different multipliers
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @move_to_two_cells
func.func @move_to_two_cells(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.shift %ptr, 1 : index -> index
    bf.modify %1, 1 : index
    %2 = bf.shift %1, 1 : index -> index
    bf.modify %2, 3 : index
    %3 = bf.shift %2, -2 : index -> index
    bf.yield %3 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: [-]  —  simple decrement loop
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @simple_decrement_loop
func.func @simple_decrement_loop(%ptr: index) -> index {
  // CHECK: memref.load
  // CHECK: arith.index_castui
  // CHECK: scf.for
  // CHECK:   scf.yield
  // CHECK: }
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: [->+<]  —  using raw left/right/add ops (before bf-combine)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @raw_ops_move
func.func @raw_ops_move(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.sub %ptr : index
    %1 = bf.right %ptr : index -> index
    bf.add %1 : index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: clear at non-zero offset inside body
//   [->[-]<]  —  decrement at 0, clear at +1
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @clear_inside_body
func.func @clear_inside_body(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.right %ptr : index -> index
    bf.clear %1 : index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Positive: offset never reaches 0 again until yield
//   [->>>+<<<]  —  move to offset +3
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @move_to_offset_3
func.func @move_to_offset_3(%ptr: index) -> index {
  // CHECK: scf.for
  // CHECK:   arith.addi
  // CHECK:   memref.load
  // CHECK:   arith.addi
  // CHECK:   memref.store
  // CHECK:   scf.yield
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.shift %ptr, 3 : index -> index
    bf.modify %1, 1 : index
    %2 = bf.shift %1, -3 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: pointer does not return to start (net offset != 0)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_return_ptr
func.func @no_return_ptr(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.right %ptr : index -> index
    bf.yield %1 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: pointer moves and returns to wrong position
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @wrong_return_offset
func.func @wrong_return_offset(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.shift %ptr, 3 : index -> index
    %2 = bf.shift %1, -2 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: nested loop
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @nested_loop
func.func @nested_loop(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    bf.loop(%ptr) {
      bf.modify %ptr, -1 : index
      bf.yield %ptr : index
    } : index -> index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: net delta not -1 (delta = -2)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @wrong_delta
func.func @wrong_delta(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -2 : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: net delta is +1 (not -1)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @wrong_sign_delta
func.func @wrong_sign_delta(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, 1 : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: no modification at ptr[0] at all
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @no_delta_at_zero
func.func @no_delta_at_zero(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    %1 = bf.right %ptr : index -> index
    bf.modify %1, 5 : index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: I/O inside loop body
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @io_inside_loop
func.func @io_inside_loop(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    bf.write %ptr : index
    bf.yield %ptr : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: loop with only shift (no cell modification)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @only_shifts
func.func @only_shifts(%ptr: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    %1 = bf.right %ptr : index -> index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}

//===----------------------------------------------------------------------===//
// Negative: SSA chain broken (shift after modify uses wrong ptr)
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @broken_ssa_chain
func.func @broken_ssa_chain(%ptr: index, %other: index) -> index {
  // CHECK: bf.loop
  %0 = bf.loop(%ptr) {
    bf.modify %ptr, -1 : index
    %1 = bf.right %other : index -> index
    bf.modify %1, 1 : index
    %2 = bf.left %1 : index -> index
    bf.yield %2 : index
  } : index -> index
  return %0 : index
}
