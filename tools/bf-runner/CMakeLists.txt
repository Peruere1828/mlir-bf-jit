set(LIBS
  # BF 库
  MLIRBfDialect
  MLIRBfParser
  MLIRBfTransforms
  MLIRBfToAffine
  
  # MLIR 标准库
  MLIRIR
  MLIROptLib
  MLIRPass
  MLIRExecutionEngine

  LLVMX86AsmParser
  
  # 降级所需的标准 Pass 库
  MLIRAffineToStandard
  MLIRSCFToControlFlow
  MLIRControlFlowToLLVM
  MLIRFuncToLLVM
  MLIRMemRefToLLVM
  MLIRLLVMToLLVMIRTranslation
  MLIRReconcileUnrealizedCasts
)

add_llvm_executable(bf-runner bf-runner.cpp)
llvm_update_compile_flags(bf-runner)
target_link_libraries(bf-runner PRIVATE ${LIBS})
mlir_check_all_link_libraries(bf-runner)