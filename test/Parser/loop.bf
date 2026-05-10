// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main() -> i32
// CHECK: bf.loop
// CHECK: bf.sub
// CHECK: bf.yield
[-]