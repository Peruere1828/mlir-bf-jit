set(LIBS
  MLIRIR
  MLIROptLib
  MLIRArithDialect
  MLIRBfDialect
  MLIRBfParser
  MLIRBfTransforms
)

add_llvm_executable(bf-opt bf-opt.cpp)
llvm_update_compile_flags(bf-opt)
target_link_libraries(bf-opt PRIVATE ${LIBS})
mlir_check_all_link_libraries(bf-opt)
