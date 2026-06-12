// RUN: bf-runner -O 2 %s 2>/dev/null | FileCheck %s
// Clear pattern [-]: cell0=5, clear to 0, output as 'A' (0+65)
+++++[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.
// CHECK: A
