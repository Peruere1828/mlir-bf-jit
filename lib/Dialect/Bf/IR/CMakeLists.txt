add_mlir_dialect_library(MLIRBfDialect
  BfDialect.cpp
  BfOps.cpp

  DEPENDS
  MLIRBfOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRSideEffectInterfaces
)
