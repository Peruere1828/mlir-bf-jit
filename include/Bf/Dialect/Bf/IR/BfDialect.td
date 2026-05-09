#ifndef BF_DIALECT
#define BF_DIALECT

include "mlir/IR/DialectBase.td"

def BfDialect : Dialect {
  let name = "bf";
  
  let summary = "The Bf dialect";
  let description = [{}];

  let cppNamespace = "::mlir::bf";

  let useDefaultAttributePrinterParser = 1;
  let useDefaultTypePrinterParser = 1;
}

#endif // BF_DIALECT
