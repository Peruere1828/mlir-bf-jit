#ifndef BF_OPS
#define BF_OPS

include "BfDialect.td"
include "mlir/IR/BuiltinAttributes.td"
include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

class Bf_Op<string mnemonic, list<Trait> traits = []> :
    Op<Bf_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Pointer movement
//===----------------------------------------------------------------------===//

def Bf_LeftOp : Bf_Op<"left"> {
  let summary = "Move pointer left by a step value";
  let arguments = (ins I32Attr:$value);
  let assemblyFormat = "$value attr-dict";
}

def Bf_RightOp : Bf_Op<"right"> {
  let summary = "Move pointer right by a step value";
  let arguments = (ins I32Attr:$value);
  let assemblyFormat = "$value attr-dict";
}

//===----------------------------------------------------------------------===//
// Cell value modification
//===----------------------------------------------------------------------===//

def Bf_AddOp : Bf_Op<"add"> {
  let summary = "Brainfuck '+' operation with a step value";
  let arguments = (ins I32Attr:$value);
  let assemblyFormat = "$value attr-dict";
}

def Bf_SubOp : Bf_Op<"sub"> {
  let summary = "Brainfuck '-' operation with a step value";
  let arguments = (ins I32Attr:$value);
  let assemblyFormat = "$value attr-dict";
}

//===----------------------------------------------------------------------===//
// I/O — NOT Pure: observable external side effects
//===----------------------------------------------------------------------===//



//===----------------------------------------------------------------------===//
// Control flow
//===----------------------------------------------------------------------===//



#endif // BF_OPS
