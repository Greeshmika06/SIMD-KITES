# SIMD Extension in KITES RISC-V Simulator

## Overview

This project extends the KITES RISC-V simulator with custom SIMD (Single Instruction, Multiple Data) instructions for packed 8-bit operations.

It uses 64-bit general-purpose registers divided into eight independent 8-bit lanes, allowing one instruction to process eight values simultaneously.

## Custom SIMD Instructions

### Unsigned Instructions

| Instruction | Function |
|---|---|
| `BADD` | Wrapping addition |
| `BSUB` | Wrapping subtraction |
| `BMIN` | Unsigned minimum |
| `BMAX` | Unsigned maximum |
| `BMUL` | Multiplication (lower 8 bits retained) |
| `BADDS` | Saturating addition |
| `BSUBS` | Saturating subtraction |
| `BMULS` | Saturating multiplication |

### Signed Instructions

| Instruction | Function |
|---|---|
| `BSADD` | Signed saturating addition |
| `BSSUB` | Signed saturating subtraction |
| `BSMUL` | Signed saturating multiplication |

Signed operations use the range -128 to 127 and process all eight lanes independently.

## Instruction Encoding

All instructions use the R-type format and custom opcode `0101011` (`0x2B`).

- Unsigned instructions: `funct7 = 0000000`
- Signed instructions: `funct7 = 0000001`

The `funct3` field selects the operation.

| funct3 | Unsigned | Signed |
|---|---|---|
| `000` | BADD | BSADD |
| `001` | BSUB | BSSUB |
| `010` | BMIN | — |
| `011` | BMAX | — |
| `100` | BMUL | BSMUL |
| `101` | BADDS | — |
| `110` | BSUBS | — |
| `111` | BMULS | — |

## Applications

The SIMD extension is demonstrated through three image-processing applications:

1. **Grayscale Brightness Adjustment:** Uses BADD and BADDS to increase pixel brightness.
2. **Image Difference Detection:** Uses BSUBS and BMAX to calculate differences between two frames.
3. **Binary-Mask Compositing:** Uses BSUB, BMUL and BADD to combine foreground and background pixels using a binary mask.

## Performance Evaluation

Scalar and SIMD implementations are compared using the same inputs and expected outputs.

Performance is measured using simulated cycle counts from KITES.

`Speedup = Scalar Cycles / SIMD Cycles`

The SIMD implementation reduces simulated execution cycles for the tested operations and applications.

## Testing

The implementation is verified using the Google Test framework.

Tests cover:
- All custom SIMD instructions.
- Overflow, underflow and saturation.
- Signed and unsigned boundary values.
- Independent processing of all eight lanes.
- Scalar reference comparisons.

## Challenges and Limitations

- Extended the initial four-lane examples and tests to support all eight lanes of the 64-bit register.
- Currently focuses on packed 8-bit operations.
- BMUL retains only the lower 8 bits, so it does not preserve full multiplication results.
- Tested in the KITES simulator, not on physical hardware.

## Future Scope

- Support for 16-bit and 32-bit SIMD operations.
- Additional signed SIMD instructions.
- Integration into a five-stage pipelined processor.
- More applications in image and audio processing.

## Building

Requires Qt 6.9 or above and CMake.

```bash
cmake -DCMAKE_PREFIX_PATH="path/to/Qt/6.x.x/<compiler>" ..
```

## Testing Documentation

Refer to [docs/TESTING.md](docs/TESTING.md) for build and test instructions.

## CLI Documentation

Refer to [docs/CLI.md](docs/CLI.md) for command-line usage.

## Original KITES Repository

https://github.com/hghit-harshit/Kites
