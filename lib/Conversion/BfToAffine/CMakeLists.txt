add_mlir_conversion_library(MLIRBfToAffine
  BfToAffine.cpp

  DEPENDS
  MLIRBfConversionPassIncGen

  LINK_LIBS PUBLIC
  MLIRBfDialect
  MLIRPass
  MLIRAffineDialect
  MLIRMemRefDialect
  MLIRSCFDialect
  MLIRArithDialect
  MLIRFuncDialect
  MLIRTransforms
)