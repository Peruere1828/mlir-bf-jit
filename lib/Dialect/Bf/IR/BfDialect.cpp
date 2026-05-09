#include "Bf/Dialect/Bf/IR/BfOps.h"

using namespace mlir;
using namespace mlir::bf;

//===----------------------------------------------------------------------===//
// Generated: BfDialect class definition skeleton
//===----------------------------------------------------------------------===//

#include "Bf/Dialect/Bf/IR/BfOpsDialect.cpp.inc"

//===----------------------------------------------------------------------===//
// BfDialect::initialize
//===----------------------------------------------------------------------===//

void BfDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Bf/Dialect/Bf/IR/BfOps.cpp.inc"
  >();
}
