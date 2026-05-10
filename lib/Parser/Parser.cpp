#include "Bf/Parser/Parser.h"
#include "Bf/Dialect/Bf/IR/BfDialect.h"
#include "Bf/Dialect/Bf/IR/BfOps.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/Operation.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

#include <cstddef>
#include <stack>
#include <utility>

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
  std::stack<std::pair<bf::LoopOp, OpBuilder::InsertPoint>> loopStack;

  auto indexType = builder.getIndexType();
  auto currentPtr = builder.create<arith::ConstantIndexOp>(loc, 0).getResult();
  auto stepOneAttr = builder.getI32IntegerAttr(1);

  for (size_t i = 0; i < source.size(); ++i) {
    if (i + 1 < source.size() && source.substr(i, 2) == "//") {
      // 把 // 视为注释，方便 lit 测试
      while (i < source.size() && source[i] != '\n') {
        i++;
      }
      continue;
    }
    char ch = source[i];
    switch (ch) {
    case '>':
      // Increment the data pointer
      currentPtr =
          builder.create<bf::RightOp>(loc, indexType, currentPtr, stepOneAttr)
              .getResult();
      break;
    case '<':
      // Decrement the data pointer
      currentPtr =
          builder.create<bf::LeftOp>(loc, indexType, currentPtr, stepOneAttr)
              .getResult();
      break;
    case '+':
      // Increment the byte at the data pointer
      builder.create<bf::AddOp>(loc, currentPtr, stepOneAttr);
      break;
    case '-':
      // Decrement the byte at the data pointer
      builder.create<bf::SubOp>(loc, currentPtr, stepOneAttr);
      break;
    case '.':
      // Output the byte at the data pointer
      builder.create<bf::WriteOp>(loc, currentPtr);
      break;
    case ',':
      // Input a byte and store it at the data pointer
      builder.create<bf::ReadOp>(loc, currentPtr);
      break;
    case '[': {
      // Start of a loop - create a new block for the loop body
      auto loopOp = builder.create<bf::LoopOp>(loc, indexType, currentPtr);
      loopStack.push(std::make_pair(loopOp, builder.saveInsertionPoint()));

      Block *loopBlock = builder.createBlock(
          &loopOp.getRegion(), loopOp.getRegion().end(), {indexType}, {loc});
      currentPtr = loopBlock->getArgument(0);
      break;
    }
    case ']': {
      // End of a loop - pop the last loop block and create a branch back to it
      if (loopStack.empty()) {
        llvm::errs() << "Error: Unmatched ']' found.\n";
        return {};
      }
      builder.create<bf::YieldOp>(loc, currentPtr);
      auto [loopOp, insertPoint] = loopStack.top();
      loopStack.pop();

      builder.restoreInsertionPoint(insertPoint);
      currentPtr = loopOp.getResult();
      break;
    }
    default:
      // Ignore any other characters (comments, whitespace, etc.)
      break;
    }
  }

  auto zero = builder.create<arith::ConstantIntOp>(loc, 0, 32);
  builder.create<func::ReturnOp>(loc, zero->getResult(0));

  if (!module) {
    return {};
  }

  return module;
}
