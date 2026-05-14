#ifndef BF_CONVERSION_PASSES
#define BF_CONVERSION_PASSES

include "mlir/Pass/PassBase.td"

def ConvertBfToAffine : Pass<"convert-bf-to-affine", "ModuleOp"> {
  let summary = "Lower bf dialect to affine/scf/arith/memref/func dialects";
  let description = [{
    Allocates a memref<30000xi8> tape, replaces bf ops with affine loads/stores,
    arithmetic ops, and scf.while loops. I/O becomes calls to extern getchar/putchar.
  }];
  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::affine::AffineDialect",
    "mlir::memref::MemRefDialect",
    "mlir::scf::SCFDialect",
    "mlir::func::FuncDialect",
  ];
}

#endif // BF_CONVERSION_PASSES