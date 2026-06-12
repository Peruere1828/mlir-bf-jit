# BrainFuck JIT (bf-jit)

A BrainFuck JIT compiler built on the LLVM/MLIR toolchain.

Based on LLVM 20.1.8.

**Conventions:**

- The tape length is 30,000 cells, numbered from 0 to 29,999.
- The value type is `uint8_t`, and arithmetic overflow wraps around modulo 256.

## Prerequisites

- LLVM/MLIR 20.1.8 (pre-built installation, headers + libraries)
- CMake >= 3.20
- Ninja (or any CMake-supported generator)
- Clang (C++17 toolchain)

The default configuration assumes MLIR is installed at `~/mlir-install`. Adjust `MLIR_DIR` and `LLVM_DIR` in the CMake invocation if yours lives elsewhere.

## Build

```bash
# Configure (from the project root)
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DMLIR_DIR=$HOME/mlir-install/lib/cmake/mlir \
  -DLLVM_DIR=$HOME/mlir-install/lib/cmake/llvm \
  -S . -B build

# Build everything
ninja -C build
```

### Build individual targets

```bash
ninja -C build bf-translate    # parser → MLIR text
ninja -C build bf-opt          # optimizer driver
ninja -C build bf-runner       # JIT runner
ninja -C build MLIRBfDialect   # tablegen + dialect library only
```

## Test

Tests use LLVM's **lit** (LLVM Integrated Tester) and live under `test/`.

There are four suites:

| Directory           | What it tests                                |
|---------------------|----------------------------------------------|
| `test/Parser/`      | `.bf` → MLIR parsing (bf-translate)          |
| `test/Dialect/Bf/`  | Dialect transforms (combine, raise-to-clear, lower-to-affine-for) |
| `test/Conversion/`  | Lowering: bf → affine → scf → llvm           |
| `test/Execution/`   | End-to-end JIT execution (bf-runner)         |

### Run the full test suite

```bash
cmake --build build --target check-bf-jit
```

This automatically rebuilds `bf-translate`, `bf-opt`, and `bf-runner` if needed, then runs every test.

### Run a subset of tests

```bash
# Run only parser tests
cd build && ./bin/llvm-lit test/Parser -v

# Run only execution tests
cd build && ./bin/llvm-lit test/Execution -v

# Run a single test file
cd build && ./bin/llvm-lit test/Execution/simple_output.bf -v
```

## Tools

### `bf-translate` — parse `.bf` to MLIR

Reads a BrainFuck source file and emits the corresponding `bf` dialect MLIR.

```bash
./build/bin/bf-translate hello.bf              # prints to stdout
./build/bin/bf-translate hello.bf -o out.mlir  # writes to file
```

### `bf-opt` — optimize `.mlir`

Loads a `bf` dialect `.mlir` file and runs transform passes on it.

```bash
# Canonicalize
./build/bin/bf-opt input.mlir --canonicalize

# Run specific bf transforms
./build/bin/bf-opt input.mlir --bf-combine
./build/bin/bf-opt input.mlir --bf-raise-to-clear
./build/bin/bf-opt input.mlir --bf-lower-to-affine-for

# Full pipeline (with -o for output)
./build/bin/bf-opt input.mlir --bf-combine --bf-raise-to-clear --canonicalize \
  --bf-lower-to-affine-for -o optimized.mlir
```

### `bf-runner` — JIT execute a `.bf` file

Compiles a `.bf` file all the way down to native code and runs it.

```bash
# Run without optimizations
./build/bin/bf-runner hello.bf

# Run with optimizations (bf-combine → raise-to-clear → affine lowering)
./build/bin/bf-runner -O2 hello.bf
```

## Lowering Pipeline

```
.bf source
  │ bf-translate
  ▼
bf dialect (high-level: bf.add, bf.loop, bf.read, …)
  │ bf-combine → bf-raise-to-clear → bf-lower-to-affine-for
  ▼
bf dialect (lowered: affine.for, affine.load, affine.store, …)
  │ convert-bf-to-affine
  ▼
affine + memref + func
  │ lower-affine
  ▼
scf + memref + func
  │ convert-scf-to-cf → convert-cf-to-llvm → convert-func-to-llvm
  ▼
arith + llvm
  │ arith-to-llvm + finalize-memref-to-llvm + reconcile-unrealized-casts
  ▼
LLVM IR → JIT (LLVM ExecutionEngine)
```

## Project Structure

```
include/Bf/
├── Conversion/       # bf → affine lowering pass
├── Dialect/Bf/
│   ├── IR/           # Dialect + Op definitions (.td, ODS)
│   └── Transforms/   # bf→bf optimization passes
├── Parser/           # .bf frontend parser
└── ExecutionEngine/  # Runtime helpers (putchar/getchar stubs)
lib/                  # Implementation (mirrors include/)
tools/
├── bf-translate/     # .bf → MLIR text
├── bf-opt/           # MLIR pass runner
└── bf-runner/        # JIT compiler + executor
test/
├── Parser/           # Parsing tests
├── Dialect/Bf/       # Transform tests
├── Conversion/       # Lowering tests
└── Execution/        # End-to-end JIT tests
```
