#ifndef BF_CONVERSION_BFTOAFFINE_BFTOAFFINE_H
#define BF_CONVERSION_BFTOAFFINE_BFTOAFFINE_H

#include "Bf/Conversion/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace bf {

class Pass;

#define GEN_PASS_DECL_CONVERTBFTOAFFINE
#include "Bf/Conversion/Passes.h.inc"

} // namespace bf
} // namespace mlir

#endif