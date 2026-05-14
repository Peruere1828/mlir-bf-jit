// RUN: bf-translate %s | FileCheck %s

// Program containing only //-style comments should produce the same output
// as an empty program

// CHECK-LABEL: func.func @main()
// CHECK: %[[C0:.*]] = arith.constant 0 : index
// CHECK-NOT: bf.
// CHECK: return
