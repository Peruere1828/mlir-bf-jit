// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main()
// CHECK-NEXT: %[[ZERO:.*]] = arith.constant 0 : index
// CHECK-DAG: bf.right
// CHECK-DAG: bf.left
// CHECK: bf.sub
><-