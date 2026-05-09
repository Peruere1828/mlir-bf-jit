#include "Bf/Parser/Parser.h"

#include "mlir/IR/AsmState.h"
#include "mlir/IR/MLIRContext.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

namespace cl = llvm::cl;

static cl::opt<std::string>
    inputFilename(cl::Positional, cl::desc("<input file>"), cl::Required);

int main(int argc, char **argv) {
  mlir::registerAsmPrinterCLOptions();
  mlir::registerMLIRContextCLOptions();
  cl::ParseCommandLineOptions(argc, argv, "BF Compiler\n");

  mlir::MLIRContext context;
  context.loadAllAvailableDialects();

  if (!llvm::StringRef(inputFilename).ends_with(".bf")) {
    llvm::errs() << "Input file must have a .bf extension\n";
    return 1;
  }

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
      llvm::MemoryBuffer::getFileOrSTDIN(inputFilename);
  if (std::error_code ec = fileOrErr.getError()) {
    llvm::errs() << "Could not open input file: " << ec.message() << "\n";
    return 1;
  }

  llvm::SourceMgr sourceMgr;
  sourceMgr.AddNewSourceBuffer(std::move(*fileOrErr), llvm::SMLoc());
  auto module = mlir::bf::parseBFSource(&context, sourceMgr);

  if (!module) {
    llvm::errs() << "Error parsing source file: " << inputFilename << "\n";
    return 1;
  }

  module->dump();
  return 0;
}