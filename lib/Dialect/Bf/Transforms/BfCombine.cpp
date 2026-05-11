#include "Bf/Dialect/Bf/IR/BfOps.h"
#include "Bf/Dialect/Bf/Transforms/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Value.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/ADT/SmallVector.h"
#include <cstdint>

namespace mlir {
namespace bf {

#define GEN_PASS_DEF_BFCOMBINE
#include "Bf/Dialect/Bf/Transforms/Passes.h.inc"

namespace {

// 功能：合并连续的指针移动 / 单元格修改为单个 bf.shift / bf.modify

// 将 left/right 操作提升为 shift 操作
template <typename OpType, int32_t kOffset>
struct LiftToShift : public OpRewritePattern<OpType> {
  using OpRewritePattern<OpType>::OpRewritePattern;

  LiftToShift(MLIRContext *context, PatternBenefit benefit = 1)
      : OpRewritePattern<OpType>(context, benefit) {}

  LogicalResult matchAndRewrite(OpType op,
                                PatternRewriter &rewriter) const override {
    auto offset = rewriter.getI32IntegerAttr(kOffset);
    rewriter.replaceOpWithNewOp<bf::ShiftOp>(op, op.getType(), op.getPtr(),
                                             offset);
    return success();
  }
};

// 将 add/sub 操作提升为 modify 操作
template <typename OpType, int32_t kDelta>
struct LiftToModify : public OpRewritePattern<OpType> {
  using OpRewritePattern<OpType>::OpRewritePattern;

  LiftToModify(MLIRContext *context, PatternBenefit benefit = 1)
      : OpRewritePattern<OpType>(context, benefit) {}

  LogicalResult matchAndRewrite(OpType op,
                                PatternRewriter &rewriter) const override {
    auto offset = rewriter.getI32IntegerAttr(kDelta);
    rewriter.replaceOpWithNewOp<bf::ModifyOp>(op, op.getPtr(), offset);
    return success();
  }
};

// 匹配连续的 ShiftOp，其中每个 ShiftOp 的输入都是前一个 ShiftOp 的输出
struct MergeConsecutiveShifts : public OpRewritePattern<bf::ShiftOp> {
  using OpRewritePattern::OpRewritePattern;

  MergeConsecutiveShifts(MLIRContext *context, PatternBenefit benefit = 1)
      : OpRewritePattern<bf::ShiftOp>(context, benefit) {}

  LogicalResult matchAndRewrite(bf::ShiftOp shiftOp,
                                PatternRewriter &rewriter) const override {
    int32_t cumulativeOffset = static_cast<int32_t>(shiftOp.getOffset());
    auto currentShift = shiftOp;
    SmallVector<bf::ShiftOp> toMerge;

    while (true) {
      auto *nextOp = currentShift->getNextNode();
      auto nextShiftOp = dyn_cast_or_null<bf::ShiftOp>(nextOp);
      if (!nextShiftOp)
        break;
      if (currentShift.getRes() != nextShiftOp.getPtr())
        break;
      if (!currentShift.getRes().hasOneUse())
        break;
      cumulativeOffset += static_cast<int32_t>(nextShiftOp.getOffset());
      toMerge.push_back(nextShiftOp);
      currentShift = nextShiftOp;
    }

    if (toMerge.empty())
      return failure();

    rewriter.replaceAllUsesWith(currentShift.getRes(), shiftOp.getRes());
    rewriter.modifyOpInPlace(shiftOp, [&]() {
      shiftOp.setOffsetAttr(rewriter.getI32IntegerAttr(cumulativeOffset));
    });
    for (auto op : toMerge) {
      rewriter.eraseOp(op);
    }
    return success();
  }
};

// 匹配连续的 ModifyOp
struct MergeConsecutiveModifies : public OpRewritePattern<bf::ModifyOp> {
  using OpRewritePattern::OpRewritePattern;

  MergeConsecutiveModifies(MLIRContext *context, PatternBenefit benefit = 1)
      : OpRewritePattern<bf::ModifyOp>(context, benefit) {}

  LogicalResult matchAndRewrite(bf::ModifyOp modifyOp,
                                PatternRewriter &rewriter) const override {
    int32_t cumulativeDelta = static_cast<int32_t>(modifyOp.getDelta());
    auto currentModify = modifyOp;
    SmallVector<bf::ModifyOp> toMerge;

    while (true) {
      auto *nextOp = currentModify->getNextNode();
      auto nextModifyOp = dyn_cast_or_null<bf::ModifyOp>(nextOp);
      if (!nextModifyOp)
        break;
      if (currentModify.getPtr() != nextModifyOp.getPtr())
        break;
      cumulativeDelta += static_cast<int32_t>(nextModifyOp.getDelta());
      toMerge.push_back(nextModifyOp);
      currentModify = nextModifyOp;
    }

    if (toMerge.empty())
      return failure();

    rewriter.modifyOpInPlace(modifyOp, [&]() {
      modifyOp.setDeltaAttr(rewriter.getI32IntegerAttr(cumulativeDelta));
    });
    for (auto op : toMerge) {
      rewriter.eraseOp(op);
    }
    return success();
  }
};

// 必须优先把所有的 left/right/add/sub 提升为 shift/modify 后，后续的 pass 才能生效
struct BfCombinePass : public impl::BfCombineBase<BfCombinePass> {
  void runOnOperation() override {
    auto func = getOperation();
    RewritePatternSet patterns(&getContext());

    patterns.add<LiftToShift<bf::LeftOp, -1>>(&getContext());
    patterns.add<LiftToShift<bf::RightOp, 1>>(&getContext());
    patterns.add<LiftToModify<bf::AddOp, 1>>(&getContext());
    patterns.add<LiftToModify<bf::SubOp, -1>>(&getContext());
    patterns.add<MergeConsecutiveShifts>(&getContext());
    patterns.add<MergeConsecutiveModifies>(&getContext());

    if (failed(applyPatternsGreedily(func, std::move(patterns))))
      return signalPassFailure();
  }
};

} // namespace

} // namespace bf
} // namespace mlir
