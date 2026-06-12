#ifndef BF_DIALECT_BF_TRANSFORMS_PASSES_TD
#define BF_DIALECT_BF_TRANSFORMS_PASSES_TD

include "mlir/Pass/PassBase.td"

def BfCombine : Pass<"bf-combine", "func::FuncOp"> {
  let summary = "Combine bf dialect ops into higher-level semantic ops";
  let description = [{
    Merge consecutive low-level pointer/cell ops into bf.shift / bf.modify
  }];
  let dependentDialects = ["mlir::bf::BfDialect"];
}

def BfRaiseToClear : Pass<"bf-raise-to-clear", "func::FuncOp"> {
  let summary = "Raise `loop modify -1` to bf.clear";
  let description = [{}];
  let dependentDialects = ["mlir::bf::BfDialect"];
}

def BfLowerToAffineFor : Pass<"bf-lower-to-affine-for", "ModuleOp"> {
  let summary = "Lower counted BF loops to affine.for with iter_args";
  let description = [{
    Recognizes bf.loop patterns where the pointer returns to its starting
    position and the net cell change at ptr[0] is exactly -1. These loops
    are converted to affine.for with affine.load / affine.store inside.
  }];
  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::memref::MemRefDialect",
    "mlir::scf::SCFDialect",
  ];
}

#endif
