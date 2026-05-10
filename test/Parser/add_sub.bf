// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main() -> i32
// CHECK: bf.add
// CHECK: bf.add
// CHECK: bf.add
// CHECK: bf.sub
+++-