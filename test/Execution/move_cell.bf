// RUN: bf-runner -O 2 %s 2>/dev/null | FileCheck %s
// Move pattern [->+<]: cell0=3 moves to cell1, then output cell1=3 as 'C' (67)
+++[->+<]>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
// CHECK: D
