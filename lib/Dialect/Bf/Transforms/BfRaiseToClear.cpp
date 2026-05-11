#include "Bf/Dialect/Bf/IR/BfOps.h"
#include "Bf/Dialect/Bf/Transforms/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Value.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include <cstdint>

namespace mlir {
namespace bf {

#define GEN_PASS_DEF_BFRAISETOCLEAR
#include "Bf/Dialect/Bf/Transforms/Passes.h.inc"

namespace {

// 匹配如下形式的 loopOp，然后将其提升为 bf.clear
// loop{ modify +1/-1 }
// loop{ bf.clear }
struct RaiseToClearPattern : public OpRewritePattern<bf::LoopOp> {
  using OpRewritePattern::OpRewritePattern;

  RaiseToClearPattern(MLIRContext *context, PatternBenefit benefit = 1)
      : OpRewritePattern<bf::LoopOp>(context, benefit) {}

  LogicalResult matchAndRewrite(bf::LoopOp loopOp,
                                PatternRewriter &rewriter) const override {
    // Implementation for raising ops to clear
    auto &block = loopOp.getRegion().front();
    auto &ops = block.getOperations();
    if (ops.size() != 2 || !isa<bf::YieldOp>(ops.back()))
      return failure();
    auto modifyOp = dyn_cast<bf::ModifyOp>(ops.front());
    if (modifyOp) {
      auto delta = static_cast<int32_t>(modifyOp.getDelta());
      if (delta != 1 && delta != -1)
        return failure();
    } else if (!isa<bf::ClearOp>(ops.front())) {
      return failure();
    }
    rewriter.create<bf::ClearOp>(loopOp.getLoc(), loopOp.getPtr());
    rewriter.replaceOp(loopOp, loopOp.getPtr());
    return success();
  }
};

struct BfRaiseToClearPass
    : public impl::BfRaiseToClearBase<BfRaiseToClearPass> {
  void runOnOperation() override {
    auto func = getOperation();
    RewritePatternSet patterns(&getContext());

    patterns.add<RaiseToClearPattern>(&getContext());

    if (failed(applyPatternsGreedily(func, std::move(patterns))))
      return signalPassFailure();
  }
};

} // namespace

} // namespace bf
} // namespace mlir
