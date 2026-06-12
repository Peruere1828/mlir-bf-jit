#include "Bf/Dialect/Bf/IR/BfOps.h"
#include "Bf/Dialect/Bf/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/ADT/DenseMap.h"
#include <cstdint>

namespace mlir {
namespace bf {

#define GEN_PASS_DEF_BFLOWERTOAFFINEFOR
#include "Bf/Dialect/Bf/Transforms/Passes.h.inc"

namespace {

static MemRefType getTapeType(MLIRContext *ctx) {
  return MemRefType::get({30000}, IntegerType::get(ctx, 8));
}

/// Recognizes bf.loop(%ptr) { body; bf.yield %ptr } where:
///   - %ptr returns to start (net offset = 0 at yield)
///   - No nested bf.loop in body
///   - Net cell delta at ptr[0] is exactly -1 (trip count driver)
///   - Body contains only cell/shift ops (no I/O, no nested loops)
///
/// Converts to:
///   %init = memref.load %tape[%ptr]
///   %N = arith.index_castui %init : i8 to index
///   memref.store %c0_i8, %tape[%ptr]
///   scf.for %iv = %c0 to %N step %c1 iter_args(%ptr_iter = %ptr) {
///     ... memref ops at computed offsets ...
///     scf.yield %ptr_iter
///   }
///
/// NOTE: We use scf.for (not affine.for) because the trip count is loaded
/// from memory and is not a valid affine symbol. Inside scf.for, we use
/// memref/arith ops because scf.for's iter_args are not valid affine dims.
struct LowerCountedLoopToAffineForPattern
    : public OpRewritePattern<bf::LoopOp> {
  using OpRewritePattern::OpRewritePattern;

  LowerCountedLoopToAffineForPattern(MLIRContext *context,
                                     PatternBenefit benefit = 1)
      : OpRewritePattern<bf::LoopOp>(context, benefit) {}

  LogicalResult matchAndRewrite(bf::LoopOp loopOp,
                                PatternRewriter &rewriter) const override {
    auto &block = loopOp.getRegion().front();
    Value entryPtr = loopOp.getPtr();

    // 1. No nested loops in body
    for (auto &op : block.without_terminator())
      if (isa<bf::LoopOp>(op))
        return failure();

    // If the loop body has a block argument (from the parser), track the
    // block argument as the initial pointer value. The ops inside the body
    // reference the block argument, not the external entryPtr.
    // If no block argument, ops capture entryPtr directly (hand-written IR).
    Value initialPtr =
        (block.getNumArguments() > 0) ? block.getArgument(0) : entryPtr;

    // 2. Walk body ops, tracking (offset from entryPtr) → delta map
    DenseMap<int64_t, int64_t> modifications;  // offset → delta
    int64_t currentOffset = 0;
    Value currentVal = initialPtr;
    bool hasIO = false;

    for (auto &op : block.without_terminator()) {
      if (auto shiftOp = dyn_cast<bf::ShiftOp>(op)) {
        if (shiftOp.getPtr() != currentVal)
          return failure();
        currentOffset += static_cast<int32_t>(shiftOp.getOffset());
        currentVal = shiftOp.getRes();
      } else if (auto leftOp = dyn_cast<bf::LeftOp>(op)) {
        if (leftOp.getPtr() != currentVal)
          return failure();
        currentOffset -= 1;
        currentVal = leftOp.getRes();
      } else if (auto rightOp = dyn_cast<bf::RightOp>(op)) {
        if (rightOp.getPtr() != currentVal)
          return failure();
        currentOffset += 1;
        currentVal = rightOp.getRes();
      } else if (auto modifyOp = dyn_cast<bf::ModifyOp>(op)) {
        if (modifyOp.getPtr() != currentVal)
          return failure();
        modifications[currentOffset] +=
            static_cast<int32_t>(modifyOp.getDelta());
      } else if (auto addOp = dyn_cast<bf::AddOp>(op)) {
        if (addOp.getPtr() != currentVal)
          return failure();
        modifications[currentOffset] += 1;
      } else if (auto subOp = dyn_cast<bf::SubOp>(op)) {
        if (subOp.getPtr() != currentVal)
          return failure();
        modifications[currentOffset] -= 1;
      } else if (isa<bf::ClearOp>(op)) {
        auto clearOp = cast<bf::ClearOp>(op);
        if (clearOp.getPtr() != currentVal)
          return failure();
        modifications[currentOffset] = kClearSentinel;
      } else if (isa<bf::ReadOp, bf::WriteOp>(op)) {
        hasIO = true;
      } else {
        return failure();
      }
    }

    if (hasIO)
      return failure();

    // 3. Pointer must return to start (net offset = 0 at yield)
    if (currentOffset != 0)
      return failure();

    // 4. Verify net delta at offset 0 is exactly -1 (trip count driver)
    if (!modifications.count(0) || modifications[0] != -1)
      return failure();
    modifications.erase(0);

    // 5. Build replacement
    auto loc = loopOp.getLoc();
    auto ctx = rewriter.getContext();
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(ctx), "bf_tape");

    // 5a. Load initial cell value as trip count
    auto initVal = rewriter.create<memref::LoadOp>(
        loc, tape, ValueRange{entryPtr});
    auto tripCount = rewriter.create<arith::IndexCastUIOp>(
        loc, rewriter.getIndexType(), initVal);

    // 5b. Store 0 at ptr[0]
    auto zeroI8 = rewriter.create<arith::ConstantIntOp>(loc, 0, 8);
    rewriter.create<memref::StoreOp>(loc, zeroI8, tape, ValueRange{entryPtr});

    // 5c. Build scf.for with lower bound 0, upper bound = trip count.
    //     We use scf.for because the trip count from memory is not a valid
    //     affine symbol. Inside the body we use arith.addi for pointer
    //     offsetting and memref ops (scf.for iter_args are not affine dims).
    auto c0 = rewriter.create<arith::ConstantIndexOp>(loc, 0);
    auto c1 = rewriter.create<arith::ConstantIndexOp>(loc, 1);

    auto forOp = rewriter.create<scf::ForOp>(
        loc, c0, tripCount, c1, ValueRange{entryPtr},
        [&](OpBuilder &b, Location bodyLoc, Value iv, ValueRange args) {
          Value ptrIter = args[0];

          for (auto &[offset, delta] : modifications) {
            Value offPtr;
            if (offset == 0) {
              offPtr = ptrIter;
            } else {
              auto deltaConst = b.create<arith::ConstantIndexOp>(
                  bodyLoc, offset);
              offPtr = b.create<arith::AddIOp>(
                  bodyLoc, ptrIter, deltaConst);
            }

            if (delta == kClearSentinel) {
              auto zero = b.create<arith::ConstantIntOp>(bodyLoc, 0, 8);
              b.create<memref::StoreOp>(
                  bodyLoc, zero, tape, ValueRange{offPtr});
            } else {
              auto oldVal = b.create<memref::LoadOp>(
                  bodyLoc, tape, ValueRange{offPtr});
              auto deltaConst = b.create<arith::ConstantIntOp>(
                  bodyLoc, delta, 8);
              auto newVal = b.create<arith::AddIOp>(
                  bodyLoc, oldVal, deltaConst);
              b.create<memref::StoreOp>(
                  bodyLoc, newVal, tape, ValueRange{offPtr});
            }
          }

          b.create<scf::YieldOp>(bodyLoc, ValueRange{ptrIter});
        });

    rewriter.replaceOp(loopOp, forOp.getResult(0));
    return success();
  }

  static constexpr int64_t kClearSentinel = INT64_MIN;
};

struct BfLowerToAffineForPass
    : public impl::BfLowerToAffineForBase<BfLowerToAffineForPass> {
  void runOnOperation() override {
    auto module = getOperation();
    auto *context = &getContext();

    // Ensure the tape global exists at module level (created once).
    {
      bool found = false;
      for (auto &op : *module.getBody()) {
        if (auto g = dyn_cast<memref::GlobalOp>(op))
          if (g.getSymName() == "bf_tape") { found = true; break; }
      }
      if (!found) {
        OpBuilder builder(context);
        auto tapeType = MemRefType::get({30000}, builder.getI8Type());
        auto zeroAttr = builder.getZeroAttr(tapeType);
        builder.setInsertionPointToStart(module.getBody());
        builder.create<memref::GlobalOp>(
            builder.getUnknownLoc(),
            "bf_tape",
            builder.getStringAttr("private"),
            tapeType,
            zeroAttr,
            /*constant=*/false,
            /*alignment=*/IntegerAttr());
      }
    }

    // Apply patterns to all functions in the module.
    RewritePatternSet patterns(context);
    patterns.add<LowerCountedLoopToAffineForPattern>(context);
    FrozenRewritePatternSet frozen(std::move(patterns));

    for (auto func : module.getOps<func::FuncOp>()) {
      if (failed(applyPatternsGreedily(func, frozen)))
        return signalPassFailure();
    }
  }
};

} // namespace

} // namespace bf
} // namespace mlir
