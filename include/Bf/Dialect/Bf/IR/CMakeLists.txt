# Generates all .inc files from BfOps.td (which includes BfDialect.td):
#   BfOps.h.inc / BfOps.cpp.inc              — op class decls/defs
#   BfOpsDialect.h.inc / BfOpsDialect.cpp.inc — dialect class decl/def
#   BfOpsTypes.h.inc / BfOpsTypes.cpp.inc     — type decls/defs (unused)
add_mlir_dialect(BfOps bf)
add_mlir_doc(BfOps Bf Dialects/ -gen-dialect-doc)
