#include "Bf/Parser/Parser.h"

#include "mlir/IR/AsmState.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ToolOutputFile.h"

#include <string>

namespace cl = llvm::cl;

static cl::opt<std::string>
    inputFilename(cl::Positional, cl::desc("<input file>"), cl::Required);

static cl::opt<std::string> outputFilename("o", cl::desc("Output file name"),
                                           cl::value_desc("filename"),
                                           cl::init("-"));

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

  std::string errorMessage;
  auto file = mlir::openInputFile(inputFilename, &errorMessage);
  if (!file) {
    llvm::errs() << "Could not open input file: " << errorMessage << "\n";
    return 1;
  }

  llvm::SourceMgr sourceMgr;
  sourceMgr.AddNewSourceBuffer(std::move(file), llvm::SMLoc());

  auto module = mlir::bf::parseBFSource(&context, sourceMgr);
  if (!module) {
    llvm::errs() << "Error parsing source file: " << inputFilename << "\n";
    return 1;
  }

  auto output = mlir::openOutputFile(outputFilename, &errorMessage);
  if (!output) {
    llvm::errs() << "Could not open output file: " << errorMessage << "\n";
    return 1;
  }

  module->print(output->os());
  output->keep();
  return 0;
}