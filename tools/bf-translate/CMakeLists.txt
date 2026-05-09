set(LIBS
  MLIRIR
  MLIROptLib

  MLIRBfDialect
  MLIRBfParser
  # MLIRBfTransforms
  # MLIRBfConversion
)

add_llvm_executable(bf-translate bf-translate.cpp)
llvm_update_compile_flags(bf-translate)
target_link_libraries(bf-translate PRIVATE ${LIBS})
mlir_check_all_link_libraries(bf-translate)