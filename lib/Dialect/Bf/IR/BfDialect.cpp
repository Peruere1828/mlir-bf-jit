#include "Bf/Dialect/Bf/IR/BfDialect.h"
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

Attribute BfDialect::parseAttribute(DialectAsmParser &parser, Type type) const {
  return {};
}

Type BfDialect::parseType(DialectAsmParser &parser) const { return {}; }

void BfDialect::printAttribute(Attribute, DialectAsmPrinter &) const {}

void BfDialect::printType(Type, DialectAsmPrinter &) const {}