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