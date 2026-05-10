#ifndef BF_OPS
#define BF_OPS

include "BfDialect.td"
include "mlir/IR/BuiltinAttributes.td"
include "mlir/IR/OpBase.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

class Bf_Op<string mnemonic, list<Trait> traits = []> :
    Op<Bf_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Pointer movement
//===----------------------------------------------------------------------===//

def Bf_LeftOp : Bf_Op<"left"> {
  let summary = "Move pointer left by a step value";
  let arguments = (ins Index:$ptr, I32Attr:$value);
  let results = (outs Index:$res);
  let assemblyFormat = [{ $ptr `,` $value attr-dict `:` type($ptr) `->` type($res) }];
}

def Bf_RightOp : Bf_Op<"right"> {
  let summary = "Move pointer right by a step value";
  let arguments = (ins Index:$ptr, I32Attr:$value);
  let results = (outs Index:$res);
  let assemblyFormat = [{ $ptr `,` $value attr-dict `:` type($ptr) `->` type($res) }];
}

//===----------------------------------------------------------------------===//
// Cell value modification
//===----------------------------------------------------------------------===//

def Bf_AddOp : Bf_Op<"add"> {
  let summary = "Brainfuck '+' operation with a step value";
  let arguments = (ins Index:$ptr, I32Attr:$value);
  let assemblyFormat = [{ $ptr `,` $value attr-dict `:` type($ptr) }];
}

def Bf_SubOp : Bf_Op<"sub"> {
  let summary = "Brainfuck '-' operation with a step value";
  let arguments = (ins Index:$ptr, I32Attr:$value);
  let assemblyFormat = [{ $ptr `,` $value attr-dict `:` type($ptr) }];
}

//===----------------------------------------------------------------------===//
// I/O — NOT Pure: observable external side effects
//===----------------------------------------------------------------------===//

def Bf_ReadOp : Bf_Op<"read"> {
  let summary = "Brainfuck ',' operation";
  let arguments = (ins Index:$ptr);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) }];
}

def Bf_WriteOp : Bf_Op<"write"> {
  let summary = "Brainfuck '.' operation";
  let arguments = (ins Index:$ptr);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) }];
}

//===----------------------------------------------------------------------===//
// Control flow
//===----------------------------------------------------------------------===//

def Bf_LoopOp : Bf_Op<"loop"> {
  let summary = "Brainfuck loop";
  let arguments = (ins Index:$ptr);
  let results = (outs Index:$res);
  let regions = (region SizedRegion<1>:$region);
  let assemblyFormat = [{
    `(` $ptr `)` $region attr-dict `:` type($ptr) `->` type($res)
  }];
}

def Bf_YieldOp : Bf_Op<"yield", [Terminator, ReturnLike]> {
  let summary = "Brainfuck yield";
  let arguments = (ins Index:$ptr);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) }];
}

#endif // BF_OPS
