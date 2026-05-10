#ifndef BF_DIALECT_BF_TRANSFORMS_PASSES_H
#define BF_DIALECT_BF_TRANSFORMS_PASSES_H

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace bf {

#define GEN_PASS_DECL
#include "Bf/Dialect/Bf/Transforms/Passes.h.inc"

std::unique_ptr<OperationPass<func::FuncOp>> createBfCombinePass();

} // namespace bf
} // namespace mlir

#endif
