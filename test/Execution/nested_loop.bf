// RUN: bf-runner -O 2 %s 2>/dev/null | FileCheck %s
// Nested loop multiply: cell0=2, cell1=3, inner loop moves cell1→cell2
// After: cell2=6, output cell2+65='G'
++[>+++[->+<]<-]>>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
// CHECK: G
