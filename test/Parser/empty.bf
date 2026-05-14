// RUN: bf-translate %s | FileCheck %s

// Empty program: only the function structure with no BF ops

// CHECK-LABEL: func.func @main()
// CHECK: %[[C0:.*]] = arith.constant 0 : index
// CHECK-NOT: bf.
// CHECK: return
