// RUN: bf-opt --convert-bf-to-affine %s | FileCheck %s

//===----------------------------------------------------------------------===//
// bf.read  →  call @bf_getchar + memref.store
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @read_op
func.func @read_op(%ptr: index) {
  // CHECK: call @bf_getchar() : () -> i8
  // CHECK: memref.get_global @bf_tape
  // CHECK: memref.store
  bf.read %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// bf.write  →  memref.load + call @bf_putchar
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @write_op
func.func @write_op(%ptr: index) {
  // CHECK: memref.get_global @bf_tape
  // CHECK: memref.load
  // CHECK: call @bf_putchar
  bf.write %ptr : index
  return
}

//===----------------------------------------------------------------------===//
// Read then write  — combined IO test
//===----------------------------------------------------------------------===//

// CHECK-LABEL: func @read_then_write
func.func @read_then_write(%ptr: index) {
  // CHECK: call @bf_getchar
  // CHECK: memref.store
  // CHECK: memref.load
  // CHECK: call @bf_putchar
  bf.read %ptr : index
  bf.write %ptr : index
  return
}
