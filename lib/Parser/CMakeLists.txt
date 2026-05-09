add_mlir_library(MLIRBfParser
  Parser.cpp

  LINK_LIBS PUBLIC
  MLIRBfDialect
  MLIRFuncDialect
  MLIRArithDialect
  MLIRIR
  MLIRSupport
)
