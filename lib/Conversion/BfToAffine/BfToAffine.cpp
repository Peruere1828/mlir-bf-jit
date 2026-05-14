#include "Bf/Conversion/BfToAffine/BfToAffine.h"
#include "Bf/Dialect/Bf/IR/BfOps.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "llvm/ADT/TypeSwitch.h"

namespace mlir {
namespace bf {

#define GEN_PASS_DEF_CONVERTBFTOAFFINE
#include "Bf/Conversion/Passes.h.inc"

namespace {

struct ConvertBfToAffinePass
    : public impl::ConvertBfToAffineBase<ConvertBfToAffinePass> {
  void runOnOperation() override {}
};

} // namespace

} // namespace bf
} // namespace mlir