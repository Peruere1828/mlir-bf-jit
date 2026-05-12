#include "Bf/Dialect/Bf/IR/BfOps.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::bf;

//===----------------------------------------------------------------------===//
// Generated: parse/print/verify/build for each op
// All automatic via TableGen assemblyFormat — no manual C++ needed.
//===----------------------------------------------------------------------===//

#define GET_OP_CLASSES
#include "Bf/Dialect/Bf/IR/BfOps.cpp.inc"

//===----------------------------------------------------------------------===//
// Bf_ShiftOp 折叠逻辑 (Folder)
//===----------------------------------------------------------------------===//

OpFoldResult ShiftOp::fold(FoldAdaptor adaptor) {
  if (getOffset() == 0)
    return getPtr();
  return {};
}

//===----------------------------------------------------------------------===//
// Bf_ModifyOp 规范化规则 (Canonicalization Pattern)
//===----------------------------------------------------------------------===//

struct RemoveZeroModifyPattern : public OpRewritePattern<ModifyOp> {
  using OpRewritePattern<ModifyOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(ModifyOp op,
                                PatternRewriter &rewriter) const override {
    if (op.getDelta() == 0) {
      rewriter.eraseOp(op);
      return success();
    }
    return failure();
  }
};

void ModifyOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                           MLIRContext *context) {
  results.add<RemoveZeroModifyPattern>(context);
}

//===----------------------------------------------------------------------===//
// Bf_ClearOp 规范化规则 (Canonicalization Pattern)
//===----------------------------------------------------------------------===//

struct RemoveRedundantClearPattern : public OpRewritePattern<bf::ClearOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(bf::ClearOp clearOp,
                                PatternRewriter &rewriter) const override {
    Operation *prevOp = clearOp->getPrevNode();

    if (!prevOp) {
      return failure();
    }

    if (auto prevClear = dyn_cast<bf::ClearOp>(prevOp)) {
      if (clearOp.getPtr() == prevClear.getPtr()) {
        rewriter.eraseOp(clearOp);
        return success();
      }
    }

    return failure();
  }
};

// 别忘了把这个 Pattern 注册到 bf.clear 的规范化钩子里
void bf::ClearOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                              MLIRContext *context) {
  results.add<RemoveRedundantClearPattern>(context);
}