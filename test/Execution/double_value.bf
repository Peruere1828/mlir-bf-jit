// RUN: bf-runner -O 2 %s 2>/dev/null | FileCheck %s
// Double value [->++<]: cell0=3, cell1=6, output cell1+65=71='G'
+++[->++<]>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
// CHECK: G
