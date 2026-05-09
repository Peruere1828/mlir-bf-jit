#ifndef BF_PARSER_PARSER_H
#define BF_PARSER_PARSER_H

#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OwningOpRef.h"
#include "llvm/Support/SourceMgr.h"

namespace mlir {
namespace bf {

/// Parse a Brainfuck source file into an MLIR module.
/// The returned module contains a single func.func @main() -> i32
/// with BF dialect ops in its body.
OwningOpRef<ModuleOp> parseBFSource(MLIRContext *context,
                                    llvm::SourceMgr &sourceMgr);

} // namespace bf
} // namespace mlir

#endif