#include "Bf/Conversion/Passes.h"
#include "Bf/Dialect/Bf/IR/BfDialect.h"
#include "Bf/Dialect/Bf/Transforms/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Transforms/Passes.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

namespace mlir {
namespace bf {
#define GEN_PASS_REGISTRATION
#include "Bf/Dialect/Bf/Transforms/Passes.h.inc"
} // namespace bf
} // namespace mlir

namespace mlir {
namespace bf {
#define GEN_PASS_REGISTRATION
#include "Bf/Conversion/Passes.h.inc"
} // namespace bf
} // namespace mlir

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<mlir::affine::AffineDialect>();
  registry.insert<mlir::arith::ArithDialect>();
  registry.insert<mlir::bf::BfDialect>();
  registry.insert<mlir::func::FuncDialect>();
  registry.insert<mlir::memref::MemRefDialect>();
  registry.insert<mlir::scf::SCFDialect>();

  mlir::bf::registerBfPasses();
  mlir::registerTransformsPasses();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "BF Optimizer\n", registry));
}