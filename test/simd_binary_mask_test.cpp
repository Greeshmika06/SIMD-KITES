#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"
#include "utils/utils.h"

namespace
{

struct BinaryMaskRun
{
    uint64_t result;
    uint64_t instructions;
    uint64_t cycles;
};

uint64_t scalarReference(uint64_t foreground, uint64_t background, uint64_t mask)
{
    uint64_t result = 0;
    for (unsigned int lane = 0; lane < 8; ++lane)
    {
        const uint64_t shift = lane * 8;
        const uint64_t foregroundByte = (foreground >> shift) & 0xFF;
        const uint64_t backgroundByte = (background >> shift) & 0xFF;
        const uint64_t maskByte = (mask >> shift) & 0xFF;

        if (maskByte > 1)
        {
            throw std::invalid_argument("Each mask byte must be 0 or 1");
        }

        const uint64_t outputByte =
            foregroundByte * maskByte + backgroundByte * (1 - maskByte);
        result |= outputByte << shift;
    }
    return result;
}

std::string binaryMaskProgram(
    uint64_t foreground,
    uint64_t background,
    uint64_t mask,
    bool useBmul)
{
    std::string source =
        "li x1, " + std::to_string(foreground) + "\n" +
        "li x2, " + std::to_string(background) + "\n" +
        "li x3, " + std::to_string(mask) + "\n";

    if (useBmul)
    {
        source +=
            "li x4, 0x0101010101010101\n"
            "bsub x5, x4, x3\n"
            "bmul x6, x1, x3\n"
            "bmul x7, x2, x5\n"
            "badd x14, x6, x7\n";
    }
    else
    {
        source += "li x14, 0\n";
        for (unsigned int lane = 0; lane < 8; ++lane)
        {
            const unsigned int shift = lane * 8;
            source +=
                "srli x8, x1, " + std::to_string(shift) + "\n"
                "andi x8, x8, 255\n"
                "srli x9, x2, " + std::to_string(shift) + "\n"
                "andi x9, x9, 255\n"
                "srli x10, x3, " + std::to_string(shift) + "\n"
                "andi x10, x10, 255\n"
                "li x11, 1\n"
                "sub x11, x11, x10\n"
                "mul x12, x8, x10\n"
                "mul x13, x9, x11\n"
                "add x12, x12, x13\n";

            if (shift != 0)
            {
                source += "slli x12, x12, " + std::to_string(shift) + "\n";
            }
            source += "or x14, x14, x12\n";
        }
    }

    return source;
}

BinaryMaskRun runBinaryMask(
    uint64_t foreground,
    uint64_t background,
    uint64_t mask,
    bool useBmul)
{
    Kites::setupVmStateDirectory();
    auto vm = std::make_unique<Kites::RVSSProcessor>();
    std::istringstream stream(
        binaryMaskProgram(foreground, background, mask, useBmul));
    Kites::AssembledProgram program = Kites::assemble(stream);
    vm->LoadProgram(program);
    vm->RunFast();

    return {
        vm->registers_.ReadGpr(14),
        vm->instructions_retired_,
        vm->cycle_s_
    };
}

void expectBinaryMask(
    uint64_t foreground,
    uint64_t background,
    uint64_t mask)
{
    const uint64_t expected = scalarReference(foreground, background, mask);
    const auto scalar = runBinaryMask(foreground, background, mask, false);
    const auto simd = runBinaryMask(foreground, background, mask, true);

    EXPECT_EQ(scalar.result, expected);
    EXPECT_EQ(simd.result, expected);
    EXPECT_EQ(simd.result, scalar.result);
}

} // namespace

TEST(PackedSimdBinaryMask, ZeroMaskSelectsBackground)
{
    constexpr uint64_t foreground = 0x0011223344556677ULL;
    constexpr uint64_t background = 0x8899AABBCCDDEEFFULL;
    expectBinaryMask(foreground, background, 0);
}

TEST(PackedSimdBinaryMask, OneMaskSelectsForeground)
{
    constexpr uint64_t foreground = 0x0011223344556677ULL;
    constexpr uint64_t background = 0x8899AABBCCDDEEFFULL;
    expectBinaryMask(foreground, background, 0x0101010101010101ULL);
}

TEST(PackedSimdBinaryMask, MixedMaskSelectsEachLaneIndependently)
{
    constexpr uint64_t foreground = 0x1020304050607080ULL;
    constexpr uint64_t background = 0xFFEEDDCCBBAA9988ULL;
    constexpr uint64_t mask = 0x0100010100010001ULL;
    expectBinaryMask(foreground, background, mask);
}

TEST(PackedSimdBinaryMask, BoundaryValuesAndAllEightLanes)
{
    expectBinaryMask(
        0x0000000000000000ULL,
        0xFFFFFFFFFFFFFFFFULL,
        0x0100010001000100ULL);
    expectBinaryMask(
        0xFFFFFFFFFFFFFFFFULL,
        0x0000000000000000ULL,
        0x0001000100010001ULL);
    expectBinaryMask(
        0x1020304050607080ULL,
        0x1020304050607080ULL,
        0x0101010101010101ULL);
}

TEST(PackedSimdBinaryMask, ScalarAndBmulPerformance)
{
    constexpr uint64_t foreground = 0x1020304050607080ULL;
    constexpr uint64_t background = 0xFFEEDDCCBBAA9988ULL;
    constexpr uint64_t mask = 0x0100010100010001ULL;
    const auto scalar = runBinaryMask(foreground, background, mask, false);
    const auto simd = runBinaryMask(foreground, background, mask, true);

    ASSERT_EQ(scalar.result, scalarReference(foreground, background, mask));
    ASSERT_EQ(simd.result, scalar.result);
    ASSERT_GT(scalar.instructions, 0);
    ASSERT_GT(simd.instructions, 0);
    ASSERT_GT(scalar.cycles, 0);
    ASSERT_GT(simd.cycles, 0);

    const double instructionReduction =
        (1.0 - static_cast<double>(simd.instructions) / scalar.instructions) * 100.0;
    const double speedup =
        static_cast<double>(scalar.cycles) / simd.cycles;

    std::cout << "\n--- Binary-Mask Compositing Performance ---\n"
              << "Scalar cycles: " << scalar.cycles << "\n"
              << "SIMD cycles: " << simd.cycles << "\n"
              << "Instruction reduction: " << instructionReduction << "%\n"
              << "Simulated speedup: " << speedup << "x\n";
}
