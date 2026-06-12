#ifndef BF_CONVERSION_PASSES
#define BF_CONVERSION_PASSES

include "mlir/Pass/PassBase.td"

def ConvertBfToAffine : Pass<"convert-bf-to-affine", "ModuleOp"> {
  let summary = "Lower bf dialect to memref/scf/arith/func dialects";
  let description = [{
    Allocates a memref<30000xi8> tape, replaces bf ops with memref loads/stores,
    arithmetic ops, and scf.while loops. I/O becomes calls to extern getchar/putchar.
  }];
  let dependentDialects = [
    "mlir::affine::AffineDialect",
    "mlir::arith::ArithDialect",
    "mlir::memref::MemRefDialect",
    "mlir::scf::SCFDialect",
    "mlir::func::FuncDialect",
  ];
}

#endif // BF_CONVERSION_PASSES