// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main()
// CHECK: bf.add
// CHECK: bf.add
// CHECK: bf.add
// CHECK: bf.sub
+++-