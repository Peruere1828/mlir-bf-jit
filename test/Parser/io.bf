// RUN: bf-translate %s | FileCheck %s

// CHECK: func.func @main()
// CHECK: bf.read
// CHECK: bf.write
,.