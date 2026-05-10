// RUN: not bf-translate %s 2>&1 | FileCheck %s

// Unmatched ']' should produce an error and exit non-zero.
// lit's 'not' inverts the exit code so the test passes when the tool fails.

// CHECK: Unmatched ']' found.
]
