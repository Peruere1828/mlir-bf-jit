#include "Bf/Conversion/Passes.h"
#include "Bf/Dialect/Bf/IR/BfDialect.h"
#include "Bf/Dialect/Bf/Transforms/Passes.h"
#include "Bf/Parser/Parser.h"
#include "Bf/ExecutionEngine/bf_runtime.h"

#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/Passes.h"
#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/ExecutionEngine/ExecutionEngine.h"
#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Transforms/Passes.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"

#include <string>

using namespace mlir;
namespace cl = llvm::cl;

static cl::opt<std::string>
    inputFilename(cl::Positional, cl::desc("<input file>"), cl::Required);
// 仅提供 O0 和 O2
static llvm::cl::opt<unsigned int>
    optLevel("O", llvm::cl::desc("Optimization level (0 or 2)"),
             llvm::cl::init(0));

int main(int argc, char **argv) {
  llvm::InitLLVM y(argc, argv);
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  registerAsmPrinterCLOptions();
  registerMLIRContextCLOptions();
  cl::ParseCommandLineOptions(argc, argv, "BF JIT Compiler\n");

  mlir::DialectRegistry registry;
  registerBuiltinDialectTranslation(registry);
  registerLLVMDialectTranslation(registry);
  arith::registerConvertArithToLLVMInterface(registry);
  registry.insert<mlir::bf::BfDialect>();

  MLIRContext context(registry);
  context.loadAllAvailableDialects();
  registerLLVMDialectTranslation(context);

  if (!llvm::StringRef(inputFilename).ends_with(".bf")) {
    llvm::errs() << "Input file must have a .bf extension\n";
    return 1;
  }

  std::string errorMessage;
  auto file = openInputFile(inputFilename, &errorMessage);
  if (!file) {
    llvm::errs() << "Could not open input file: " << errorMessage << "\n";
    return 1;
  }

  llvm::SourceMgr sourceMgr;
  sourceMgr.AddNewSourceBuffer(std::move(file), llvm::SMLoc());

  auto module = bf::parseBFSource(&context, sourceMgr);
  if (!module) {
    llvm::errs() << "Error parsing source file: " << inputFilename << "\n";
    return 1;
  }

  PassManager pm(&context);
  if (optLevel == 0) {
    // O0 选项: builtin.module(convert-bf-to-affine)
    pm.addPass(bf::createConvertBfToAffine());
  } else if (optLevel == 2) {
    // O2 选项: builtin.module(func.func(canonicalize, bf-combine,
    // bf-raise-to-clear, canonicalize), convert-bf-to-affine)
    OpPassManager &funcPm = pm.nest<func::FuncOp>();
    funcPm.addPass(createCanonicalizerPass());
    funcPm.addPass(bf::createBfCombine());
    funcPm.addPass(bf::createBfRaiseToClear());
    funcPm.addPass(createCanonicalizerPass());

    pm.addPass(bf::createConvertBfToAffine());
  } else {
    llvm::errs() << "Invalid optimization level: " << optLevel
                 << ". Only 0 and 2 are supported.\n";
    return 1;
  }

  pm.addPass(createLowerAffinePass());
  pm.addPass(createConvertSCFToCFPass());
  pm.addPass(createConvertControlFlowToLLVMPass());
  pm.addPass(createConvertFuncToLLVMPass());
  pm.addPass(createArithToLLVMConversionPass());
  pm.addPass(createFinalizeMemRefToLLVMConversionPass());
  pm.addPass(createReconcileUnrealizedCastsPass());

  if (failed(pm.run(*module))) {
    llvm::errs() << "Error optimizing source file: " << inputFilename << "\n";
    return 1;
  }

  auto optTransformer =
      makeOptimizingTransformer(optLevel, /*sizeLevel=*/0, nullptr);
  ExecutionEngineOptions engineOptions;
  engineOptions.transformer = optTransformer;
  engineOptions.jitCodeGenOptLevel = (optLevel == 0)
                                         ? llvm::CodeGenOptLevel::None
                                         : llvm::CodeGenOptLevel::Default;
  auto maybeEngine = ExecutionEngine::create(*module, engineOptions);
  if (!maybeEngine) {
    llvm::errs() << "Failed to create ExecutionEngine: "
                 << maybeEngine.takeError() << "\n";
    return 1;
  }

  maybeEngine.get()->registerSymbols([&](llvm::orc::MangleAndInterner interner) {
    llvm::orc::SymbolMap symbolMap;
    symbolMap[interner("bf_putchar")] =
        llvm::orc::ExecutorSymbolDef::fromPtr(bf_putchar);
    symbolMap[interner("bf_getchar")] =
        llvm::orc::ExecutorSymbolDef::fromPtr(bf_getchar);
    return symbolMap;
  });

  // 提供返回槽：void 函数会忽略，非 void 函数通过它存返回值
  int32_t result = 0;
  void *returnSlot = &result;
  llvm::SmallVector<void *> args = {returnSlot};
  auto error = maybeEngine.get()->invokePacked("main", args);
  return error ? 1 : 0;
}