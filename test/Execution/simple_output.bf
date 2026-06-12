// RUN: bf-runner -O 2 %s 2>/dev/null | FileCheck %s
// 65 plus signs = 'A'
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
// CHECK: A
