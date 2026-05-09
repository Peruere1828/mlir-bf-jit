#include "Bf/Parser/Parser.h"
#include "Bf/Dialect/Bf/IR/BfDialect.h"
#include "Bf/Dialect/Bf/IR/BfOps.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Location.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

#include <stack>

using namespace mlir;
using namespace mlir::bf;

OwningOpRef<ModuleOp> mlir::bf::parseBFSource(MLIRContext *context,
                                              llvm::SourceMgr &sourceMgr) {
  context->getOrLoadDialect<bf::BfDialect>();
  context->getOrLoadDialect<func::FuncDialect>();
  context->getOrLoadDialect<arith::ArithDialect>();

  OpBuilder builder(context);
  Location loc = builder.getUnknownLoc();

  OwningOpRef<ModuleOp> module = ModuleOp::create(loc);
  builder.setInsertionPointToEnd(module->getBody());

  // 创建int main()函数
  auto funcType =
      FunctionType::get(context, {}, {IntegerType::get(context, 32)});
  auto mainFunc = builder.create<func::FuncOp>(loc, "main", funcType);

  Block *entryBlock = mainFunc.addEntryBlock();
  builder.setInsertionPointToStart(entryBlock);

  llvm::StringRef source =
      sourceMgr.getMemoryBuffer(sourceMgr.getMainFileID())->getBuffer();

  auto stepOneAttr = builder.getI32IntegerAttr(1);

  for (char ch : source) {
    switch (ch) {
    case '>':
      // Increment the data pointer
      builder.create<bf::RightOp>(loc, stepOneAttr);
      break;
    case '<':
      // Decrement the data pointer
      builder.create<bf::LeftOp>(loc, stepOneAttr);
      break;
    case '+':
      // Increment the byte at the data pointer
      builder.create<bf::AddOp>(loc, stepOneAttr);
      break;
    case '-':
      // Decrement the byte at the data pointer
      builder.create<bf::SubOp>(loc, stepOneAttr);
      break;
    case '.':
      // Output the byte at the data pointer
      break;
    case ',':
      // Input a byte and store it at the data pointer
      break;
    case '[':
      // Start of a loop - create a new block for the loop body
      break;
    case ']':
      // End of a loop - pop the last loop block and create a branch back to it
      break;
    default:
      // Ignore any other characters (comments, whitespace, etc.)
      break;
    }
  }

  auto zero =
      builder.create<arith::ConstantIntOp>(loc, 0, 32);
  builder.create<func::ReturnOp>(loc, zero->getResult(0));

  if (!module) {
    return {};
  }

  return module;
}
