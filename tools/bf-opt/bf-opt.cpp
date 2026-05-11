#include "Bf/Dialect/Bf/IR/BfDialect.h"
#include "Bf/Dialect/Bf/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

namespace mlir {
namespace bf {
#define GEN_PASS_REGISTRATION
#include "Bf/Dialect/Bf/Transforms/Passes.h.inc"
} // namespace bf
} // namespace mlir

int main(int argc, char **argv) {
  mlir::bf::registerBfPasses();

  mlir::DialectRegistry registry;
  registry.insert<mlir::arith::ArithDialect>();
  registry.insert<mlir::bf::BfDialect>();
  registry.insert<mlir::func::FuncDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "BF Optimizer\n", registry));
}