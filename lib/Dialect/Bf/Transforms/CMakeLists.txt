add_mlir_dialect_library(MLIRBfTransforms
  BfCombine.cpp
  BfLowerToAffineFor.cpp
  BfRaiseToClear.cpp

  DEPENDS
  MLIRBfPassIncGen

  LINK_LIBS PUBLIC
  MLIRBfDialect
  MLIRMemRefDialect
  MLIRArithDialect
  MLIRSCFDialect
  MLIRFuncDialect
  MLIRTransforms
  MLIRPass
)
