// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main()
// CHECK: bf.loop
// CHECK: bf.loop
// CHECK: bf.add
// CHECK: bf.yield
// CHECK: bf.yield
[[+]]