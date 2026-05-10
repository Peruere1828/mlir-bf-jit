#ifndef BF_DIALECT_BF_IR_BFOPS_H
#define BF_DIALECT_BF_IR_BFOPS_H

#include "Bf/Dialect/Bf/IR/BfDialect.h"

#include "Bf/Dialect/Bf/IR/BfDialect.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/LoopLikeInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

//===----------------------------------------------------------------------===//
// Generated op class declarations:
//   LeftOp, RightOp, AddOp, SubOp
//===----------------------------------------------------------------------===//

#define GET_OP_CLASSES
#include "Bf/Dialect/Bf/IR/BfOps.h.inc"

#endif // BF_DIALECT_BF_IR_BFOPS_H
