add_mlir_dialect_library(MLIRBfTransforms
  BfCombine.cpp
  BfRaiseToClear.cpp

  DEPENDS
  MLIRBfPassIncGen

  LINK_LIBS PUBLIC
  MLIRBfDialect
  MLIRFuncDialect
  MLIRTransforms
  MLIRPass
)
