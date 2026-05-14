// RUN: bf-translate %s | FileCheck %s

// Typical hello world pattern -- verifies a mix of pointer moves, cell ops,
// loops, and I/O all parse correctly

// CHECK: func.func @main()
// CHECK: bf.add
// CHECK: bf.loop
// CHECK: bf.left
// CHECK: bf.add
// CHECK: bf.right
// CHECK: bf.sub
// CHECK: bf.yield
// CHECK: bf.right
// CHECK: bf.add
// CHECK: bf.write
++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.