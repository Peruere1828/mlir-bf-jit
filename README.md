BrainFuck JIT using MLIR toolchain.

based on LLVM 20.1.8

tools:

- `bf-translate`: read a `.bf` file and translate it into bf dialect
- `bf-opt`: read a bf dialect `.mlir` file and optimize it
- `bf-runner`: read a `.bf` file and JIT it

conventions:

- The tape length is 30000 cells, numbered from 0 to 29999.
- The value type is `uint8_t`, and arithmetic overflow wraps around modulo 256.