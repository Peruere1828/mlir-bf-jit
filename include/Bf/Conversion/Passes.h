#ifndef BF_CONVERSION_PASSES_H
#define BF_CONVERSION_PASSES_H

#include "mlir/Pass/Pass.h"

namespace mlir {
namespace bf {

#define GEN_PASS_DECL
#include "Bf/Conversion/Passes.h.inc"

} // namespace bf
} // namespace mlir

#endif