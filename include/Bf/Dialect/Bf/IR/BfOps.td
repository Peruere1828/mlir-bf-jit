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
// Low-level pointer movement (single step, no attr)
//===----------------------------------------------------------------------===//

def Bf_LeftOp : Bf_Op<"left"> {
  let summary = "Move pointer left by one cell";
  let arguments = (ins Index:$ptr);
  let results = (outs Index:$res);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) `->` type($res) }];
}

def Bf_RightOp : Bf_Op<"right"> {
  let summary = "Move pointer right by one cell";
  let arguments = (ins Index:$ptr);
  let results = (outs Index:$res);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) `->` type($res) }];
}

//===----------------------------------------------------------------------===//
// Low-level cell modification (single step, no attr)
//===----------------------------------------------------------------------===//

def Bf_AddOp : Bf_Op<"add"> {
  let summary = "Brainfuck '+' operation (increment cell by 1)";
  let arguments = (ins Index:$ptr);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) }];
}

def Bf_SubOp : Bf_Op<"sub"> {
  let summary = "Brainfuck '-' operation (decrement cell by 1)";
  let arguments = (ins Index:$ptr);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) }];
}

//===----------------------------------------------------------------------===//
// I/O
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

//===----------------------------------------------------------------------===//
// High-level ops — produced by transforms, not by the parser
//===----------------------------------------------------------------------===//

def Bf_ShiftOp : Bf_Op<"shift"> {
  let summary = "Cumulative pointer movement (positive = right, negative = left)";
  let arguments = (ins Index:$ptr, I32Attr:$offset);
  let results = (outs Index:$res);
  let assemblyFormat = [{ $ptr `,` $offset attr-dict `:` type($ptr) `->` type($res) }];
}

def Bf_ModifyOp : Bf_Op<"modify"> {
  let summary = "Cumulative cell modification (positive = add, negative = sub)";
  let arguments = (ins Index:$ptr, I32Attr:$delta);
  let assemblyFormat = [{ $ptr `,` $delta attr-dict `:` type($ptr) }];
}

def Bf_ClearOp : Bf_Op<"clear"> {
  let summary = "Set cell to zero — equivalent to [-]";
  let arguments = (ins Index:$ptr);
  let assemblyFormat = [{ $ptr attr-dict `:` type($ptr) }];
}

def Bf_MoveAddOp : Bf_Op<"move_add"> {
  let summary = "tape[ptr + offset] += tape[ptr]; tape[ptr] = 0";
  let arguments = (ins Index:$ptr, I32Attr:$offset);
  let assemblyFormat = [{ $ptr `,` $offset attr-dict `:` type($ptr) }];
}

def Bf_MulAddOp : Bf_Op<"mul_add"> {
  let summary = "tape[ptr + offset] += tape[ptr] * multiplier; tape[ptr] = 0";
  let arguments = (ins Index:$ptr, I32Attr:$offset, I32Attr:$multiplier);
  let assemblyFormat = [{ $ptr `,` $offset `,` $multiplier attr-dict `:` type($ptr) }];
}

#endif // BF_OPS
