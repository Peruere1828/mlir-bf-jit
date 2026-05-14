// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main()
// CHECK: bf.loop
// CHECK: bf.sub
// CHECK: bf.yield
[-]