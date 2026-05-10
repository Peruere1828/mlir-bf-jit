// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main() -> i32
// CHECK: bf.read
// CHECK: bf.write
,.