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

#endif
