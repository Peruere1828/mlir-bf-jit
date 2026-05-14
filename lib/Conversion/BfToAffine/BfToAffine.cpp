#include "Bf/Conversion/BfToAffine/BfToAffine.h"
#include "Bf/Dialect/Bf/IR/BfOps.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
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

// 单元格操作，包括 clear/add/sub/modify，转换为 affine.load/affine.store
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
      rewriter.create<affine::AffineStoreOp>(loc, zero, tape,
                                             ValueRange{adaptor.getPtr()});
    } else {
      int32_t delta = 0;
      if constexpr (std::is_same_v<BfOp, bf::AddOp>)
        delta = 1;
      else if constexpr (std::is_same_v<BfOp, bf::SubOp>)
        delta = -1;
      else if constexpr (std::is_same_v<BfOp, bf::ModifyOp>)
        delta = op.getDelta();
      auto val = rewriter.create<affine::AffineLoadOp>(
          loc, tape, ValueRange{adaptor.getPtr()});
      auto deltaConst = rewriter.create<arith::ConstantIntOp>(loc, delta, 8);
      auto res = rewriter.create<arith::AddIOp>(loc, val, deltaConst);
      rewriter.create<affine::AffineStoreOp>(loc, res, tape,
                                             ValueRange{adaptor.getPtr()});
    }
    rewriter.eraseOp(op);
    return success();
  }
};

template <typename BfOp>
struct CellPtrOpConversion : public OpConversionPattern<BfOp> {
  using OpConversionPattern<BfOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(BfOp op, typename BfOp::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(op.getContext()), "bf_tape");
    auto ptr = rewriter.create<affine::AffineLoadOp>(
        loc, tape, ValueRange{adaptor.getPtr()});
    rewriter.replaceOp(op, ptr);
    return success();
  }
};

// 指针移动操作，包括 left/right/shift，转换为 affine.apply
template <typename BfOp>
struct ShiftConversion : public OpConversionPattern<BfOp> {
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
    auto map = AffineMap::get(1, 0, rewriter.getAffineDimExpr(0) + offset);
    rewriter.replaceOpWithNewOp<affine::AffineApplyOp>(op, map,
                                                       adaptor.getPtr());
    return success();
  }
};

// IO 操作，包括 getchar/putchar，转换为 func.call
struct ReadConversion : public OpConversionPattern<bf::ReadOp> {
  using OpConversionPattern::OpConversionPattern;
  LogicalResult
  matchAndRewrite(bf::ReadOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto funcCall = rewriter.create<func::CallOp>(
        loc, "bf_getchar", TypeRange{rewriter.getI8Type()}, ValueRange{});
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(op.getContext()), "bf_tape");
    rewriter.create<affine::AffineStoreOp>(loc, funcCall.getResult(0), tape,
                                           ValueRange{adaptor.getPtr()});
    rewriter.eraseOp(op);
    return success();
  }
};

struct WriteConversion : public OpConversionPattern<bf::WriteOp> {
  using OpConversionPattern::OpConversionPattern;
  LogicalResult
  matchAndRewrite(bf::WriteOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto tape = rewriter.create<memref::GetGlobalOp>(
        loc, getTapeType(op.getContext()), "bf_tape");
    auto val = rewriter.create<affine::AffineLoadOp>(
        loc, tape, ValueRange{adaptor.getPtr()});
    rewriter.create<func::CallOp>(loc, "bf_putchar", TypeRange{},
                                  ValueRange{val});
    rewriter.eraseOp(op);
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
          "bf_tape",                        // sym_name
          builder.getStringAttr("private"), // sym_visibility
          tapeType,                         // type
          zeroAttr,                         // initial_value
          false,                            // constant
          IntegerAttr()                     // alignment
      );
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
    target.addLegalDialect<affine::AffineDialect, memref::MemRefDialect,
                           scf::SCFDialect, arith::ArithDialect,
                           func::FuncDialect>();
    target.addIllegalDialect<bf::BfDialect>();

    RewritePatternSet patterns(context);
    patterns.add<CellOpConversion<bf::ClearOp>>(context);
    patterns.add<CellOpConversion<bf::AddOp>>(context);
    patterns.add<CellOpConversion<bf::SubOp>>(context);
    patterns.add<CellOpConversion<bf::ModifyOp>>(context);
    patterns.add<ShiftConversion<bf::LeftOp>>(context);
    patterns.add<ShiftConversion<bf::RightOp>>(context);
    patterns.add<ShiftConversion<bf::ShiftOp>>(context);
    patterns.add<ReadConversion>(context);
    patterns.add<WriteConversion>(context);

    if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

} // namespace bf
} // namespace mlir