// RUN: bf-translate %s | FileCheck %s

// Verify pointer SSA chain: each pointer-mutating op produces a new SSA value
// that is consumed by the next operation.

// CHECK-LABEL: func.func @main() -> i32
// CHECK: %[[INIT:.*]] = arith.constant 0 : index
// CHECK-NEXT: %[[R1:.*]] = bf.right %[[INIT]] : index -> index
// CHECK-NEXT: bf.add %[[R1]] : index
// CHECK-NEXT: %[[R2:.*]] = bf.right %[[R1]] : index -> index
// CHECK-NEXT: %[[L1:.*]] = bf.left %[[R2]] : index -> index
// CHECK-NEXT: bf.write %[[L1]]
>+><.
