#include "Bf/Conversion/BfToAffine/BfToAffine.h"
#include "Bf/Dialect/Bf/IR/BfOps.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cstdint>

namespace mlir {
namespace bf {

#define GEN_PASS_DEF_CONVERTBFTOAFFINE
#include "Bf/Conversion/Passes.h.inc"

namespace {

static MemRefType getTapeType(MLIRContext *ctx) {
  return MemRefType::get({30000}, IntegerType::get(ctx, 8));
}

// 单元格操作，转换为 memref.load/memref.store
// 用 memref 而非 affine，因为指针可能来自 scf.while（动态值），不符合
// affine 的 dimension/symbol 约束。后续可通过独立 pass 将符合条件的
// memref 访问提升为 affine 以启用多面体优化。
template <typename BfOp>
struct CellOpConversion : public OpConversionPattern<BfOp> {
  using OpConversionPattern<BfOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(BfOp op, typename BfOp::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(op.getContext()), "bf_tape");

    if constexpr (std::is_same_v<BfOp, bf::ClearOp>) {
      auto zero = rewriter.create<arith::ConstantIntOp>(loc, 0, 8);
      rewriter.create<memref::StoreOp>(loc, zero, tape,
                                       ValueRange{adaptor.getPtr()});
    } else {
      int32_t delta = 0;
      if constexpr (std::is_same_v<BfOp, bf::AddOp>)
        delta = 1;
      else if constexpr (std::is_same_v<BfOp, bf::SubOp>)
        delta = -1;
      else if constexpr (std::is_same_v<BfOp, bf::ModifyOp>)
        delta = op.getDelta();
      auto val = rewriter.create<memref::LoadOp>(
          loc, tape, ValueRange{adaptor.getPtr()});
      auto deltaConst = rewriter.create<arith::ConstantIntOp>(loc, delta, 8);
      auto res = rewriter.create<arith::AddIOp>(loc, val, deltaConst);
      rewriter.create<memref::StoreOp>(loc, res, tape,
                                       ValueRange{adaptor.getPtr()});
    }
    rewriter.eraseOp(op);
    return success();
  }
};

// 指针移动操作，转换为 arith.addi
// 不用 affine.apply，原因同上——指针可能来自 scf.while 块参数。
template <typename BfOp>
struct ShiftOpConversion : public OpConversionPattern<BfOp> {
  using OpConversionPattern<BfOp>::OpConversionPattern;
  LogicalResult
  matchAndRewrite(BfOp op, typename BfOp::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    int32_t offset = 0;
    if constexpr (std::is_same_v<BfOp, bf::RightOp>)
      offset = 1;
    else if constexpr (std::is_same_v<BfOp, bf::LeftOp>)
      offset = -1;
    else if constexpr (std::is_same_v<BfOp, bf::ShiftOp>)
      offset = op.getOffset();
    auto deltaConst = rewriter.create<arith::ConstantIndexOp>(op.getLoc(), offset);
    rewriter.replaceOpWithNewOp<arith::AddIOp>(op, adaptor.getPtr(), deltaConst);
    return success();
  }
};

// IO 操作，转换为 func.call + memref load/store
struct ReadOpConversion : public OpConversionPattern<bf::ReadOp> {
  using OpConversionPattern::OpConversionPattern;
  LogicalResult
  matchAndRewrite(bf::ReadOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto funcCall = rewriter.create<func::CallOp>(
        loc, "bf_getchar", TypeRange{rewriter.getI8Type()}, ValueRange{});
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(op.getContext()), "bf_tape");
    rewriter.create<memref::StoreOp>(loc, funcCall.getResult(0), tape,
                                     ValueRange{adaptor.getPtr()});
    rewriter.eraseOp(op);
    return success();
  }
};

struct WriteOpConversion : public OpConversionPattern<bf::WriteOp> {
  using OpConversionPattern::OpConversionPattern;
  LogicalResult
  matchAndRewrite(bf::WriteOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(op.getContext()), "bf_tape");
    auto val = rewriter.create<memref::LoadOp>(
        loc, tape, ValueRange{adaptor.getPtr()});
    rewriter.create<func::CallOp>(loc, "bf_putchar", TypeRange{},
                                  ValueRange{val});
    rewriter.eraseOp(op);
    return success();
  }
};

// Loop 操作，转换为 scf.while。BF 循环是动态的（取决于纸带当前单元的值），
// 不能用 affine.for（它要求编译期常量边界）。
struct LoopOpConversion : public OpConversionPattern<bf::LoopOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bf::LoopOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto ptrType = op.getPtr().getType();

    auto whileOp = rewriter.create<scf::WhileOp>(
        loc, TypeRange{ptrType}, adaptor.getOperands());

    // Before 区域：检查 *ptr != 0
    Block *beforeBlock = rewriter.createBlock(
        &whileOp.getBefore(), whileOp.getBefore().end(), {ptrType}, {loc});
    rewriter.setInsertionPointToStart(beforeBlock);

    Value currentPtr = beforeBlock->getArgument(0);
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(rewriter.getContext()), "bf_tape");
    Value val = rewriter.create<memref::LoadOp>(loc, tape, ValueRange{currentPtr});
    Value zero = rewriter.create<arith::ConstantIntOp>(loc, 0, 8);
    Value cond = rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::ne,
                                                val, zero);
    rewriter.create<scf::ConditionOp>(loc, cond, ValueRange{currentPtr});

    // After 区域：执行循环体
    // Body 块可能有参数（parser 生成）或直接用外层 SSA 值（手写 mlir）。
    {
      auto &loopBlock = op.getRegion().front();
      IRMapping mapping;
      Block *afterBlock = rewriter.createBlock(&whileOp.getAfter());
      Value iterArg = afterBlock->addArgument(ptrType, loc);

      if (loopBlock.getNumArguments() > 0)
        mapping.map(loopBlock.getArgument(0), iterArg);
      mapping.map(op.getPtr(), iterArg);

      for (auto &bodyOp : loopBlock.without_terminator())
        rewriter.clone(bodyOp, mapping);

      auto yieldOp = cast<bf::YieldOp>(loopBlock.back());
      Value mappedPtr = mapping.lookupOrNull(yieldOp.getPtr());
      if (!mappedPtr)
        mappedPtr = mapping.lookupOrNull(op.getPtr());
      assert(mappedPtr && "yield ptr not in mapping");
      rewriter.create<scf::YieldOp>(loc, ValueRange{mappedPtr});
    }

    rewriter.replaceOp(op, whileOp.getResult(0));
    return success();
  }
};

struct YieldOpConversion : public OpConversionPattern<bf::YieldOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(bf::YieldOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<scf::YieldOp>(op, adaptor.getOperands());
    return success();
  }
};

struct ConvertBfToAffinePass
    : public impl::ConvertBfToAffineBase<ConvertBfToAffinePass> {
  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *context = &getContext();
    OpBuilder builder(context);

    // 初始化纸带
    auto tapeType = MemRefType::get({30000}, builder.getI8Type());
    auto zeroAttr = builder.getZeroAttr(tapeType);
    if (!module.lookupSymbol("bf_tape")) {
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

    // 初始化内置函数
    if (!module.lookupSymbol("bf_getchar")) {
      builder.setInsertionPointToStart(module.getBody());
      auto funcType = builder.getFunctionType({}, {builder.getI8Type()});
      builder
          .create<func::FuncOp>(builder.getUnknownLoc(), "bf_getchar", funcType)
          .setPrivate();
    }
    if (!module.lookupSymbol("bf_putchar")) {
      builder.setInsertionPointToStart(module.getBody());
      auto funcType = builder.getFunctionType({builder.getI8Type()}, {});
      builder
          .create<func::FuncOp>(builder.getUnknownLoc(), "bf_putchar", funcType)
          .setPrivate();
    }

    ConversionTarget target(*context);
    target.addLegalDialect<memref::MemRefDialect, scf::SCFDialect,
                           arith::ArithDialect, func::FuncDialect>();
    target.addIllegalDialect<bf::BfDialect>();

    RewritePatternSet patterns(context);
    patterns.add<CellOpConversion<bf::ClearOp>>(context);
    patterns.add<CellOpConversion<bf::AddOp>>(context);
    patterns.add<CellOpConversion<bf::SubOp>>(context);
    patterns.add<CellOpConversion<bf::ModifyOp>>(context);
    patterns.add<ShiftOpConversion<bf::LeftOp>>(context);
    patterns.add<ShiftOpConversion<bf::RightOp>>(context);
    patterns.add<ShiftOpConversion<bf::ShiftOp>>(context);
    patterns.add<ReadOpConversion>(context);
    patterns.add<WriteOpConversion>(context);
    patterns.add<LoopOpConversion>(context);
    patterns.add<YieldOpConversion>(context);

    if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

} // namespace bf
} // namespace mlir
