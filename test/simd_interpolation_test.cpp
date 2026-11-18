#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"
#include "utils/utils.h"

namespace
{

struct InterpolationRun
{
    uint64_t result;
    uint64_t instructions;
    uint64_t cycles;
};

uint64_t repeatByte(uint8_t value)
{
    uint64_t packed = 0;
    for (unsigned int lane = 0; lane < 8; ++lane)
    {
        packed |= static_cast<uint64_t>(value) << (lane * 8);
    }
    return packed;
}

uint64_t scalarReference(uint64_t a, uint64_t b, uint8_t t)
{
    uint64_t result = 0;
    for (unsigned int lane = 0; lane < 8; ++lane)
    {
        const uint32_t start = (a >> (lane * 8)) & 0xFF;
        const uint32_t end = (b >> (lane * 8)) & 0xFF;
        const uint32_t weighted =
            start * (255 - t) + end * static_cast<uint32_t>(t);
        const uint32_t interpolated = (weighted + 127) / 255;
        result |= static_cast<uint64_t>(interpolated) << (lane * 8);
    }
    return result;
}

std::string interpolationProgram(uint64_t a, uint64_t b, uint8_t t, bool useBmul)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n" +
        "li x3, " + std::to_string(repeatByte(t)) + "\n";

    if (useBmul)
    {
        source +=
            "li x4, -1\n"
            "bsub x4, x4, x3\n"
            "bmul x5, x1, x4\n"
            "bmul x6, x2, x3\n";
    }

    source +=
        "li x14, 0\n"
        "li x18, 255\n";

    for (unsigned int lane = 0; lane < 8; ++lane)
    {
        const unsigned int shift = lane * 8;
        source +=
            "srli x7, x1, " + std::to_string(shift) + "\n"
            "andi x7, x7, 255\n"
            "srli x8, x2, " + std::to_string(shift) + "\n"
            "andi x8, x8, 255\n"
            "srli x9, x3, " + std::to_string(shift) + "\n"
            "andi x9, x9, 255\n"
            "li x10, 255\n"
            "sub x10, x10, x9\n"
            "mul x11, x7, x10\n"
            "mul x13, x8, x9\n";

        if (useBmul)
        {
            source +=
                "andi x11, x11, -256\n"
                "srli x12, x5, " + std::to_string(shift) + "\n"
                "andi x12, x12, 255\n"
                "or x11, x11, x12\n"
                "andi x13, x13, -256\n"
                "srli x15, x6, " + std::to_string(shift) + "\n"
                "andi x15, x15, 255\n"
                "or x13, x13, x15\n";
        }

        source +=
            "add x17, x11, x13\n"
            "addi x17, x17, 127\n"
            "divu x17, x17, x18\n"
            "slli x17, x17, " + std::to_string(shift) + "\n"
            "or x14, x14, x17\n";
    }

    return source;
}

InterpolationRun runInterpolation(uint64_t a, uint64_t b, uint8_t t, bool useBmul)
{
    Kites::setupVmStateDirectory();
    auto vm = std::make_unique<Kites::RVSSProcessor>();
    std::istringstream stream(interpolationProgram(a, b, t, useBmul));
    Kites::AssembledProgram program = Kites::assemble(stream);
    vm->LoadProgram(program);
    vm->RunFast();

    return {
        vm->registers_.ReadGpr(14),
        vm->instructions_retired_,
        vm->cycle_s_
    };
}

void expectInterpolation(uint64_t a, uint64_t b, uint8_t t)
{
    const uint64_t expected = scalarReference(a, b, t);
    const auto scalar = runInterpolation(a, b, t, false);
    const auto simd = runInterpolation(a, b, t, true);

    EXPECT_EQ(scalar.result, expected);
    EXPECT_EQ(simd.result, expected);
    EXPECT_EQ(simd.result, scalar.result);
}

} // namespace

TEST(PackedSimdInterpolation, FactorZero)
{
    constexpr uint64_t a = 0x0706050403020100ULL;
    constexpr uint64_t b = 0xFFEEDDCCBBAA9988ULL;
    expectInterpolation(a, b, 0);
}

TEST(PackedSimdInterpolation, FactorOneHalf)
{
    constexpr uint64_t a = 0x50463C32281E140AULL;
    constexpr uint64_t b = 0xC8000A141E283250ULL;

    expectInterpolation(a, b, 128);
    EXPECT_EQ(scalarReference(10, 50, 128), 30);
}

TEST(PackedSimdInterpolation, FactorOne)
{
    constexpr uint64_t a = 0x0706050403020100ULL;
    constexpr uint64_t b = 0xFFEEDDCCBBAA9988ULL;
    expectInterpolation(a, b, 255);
}

TEST(PackedSimdInterpolation, EightIndependentValuesAndBoundaries)
{
    expectInterpolation(0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL, 128);
    expectInterpolation(0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL, 128);
    expectInterpolation(0x1122334455667788ULL, 0x8877665544332211ULL, 73);
    expectInterpolation(0xA5A5A5A5A5A5A5A5ULL, 0xA5A5A5A5A5A5A5A5ULL, 128);
}

TEST(PackedSimdInterpolation, ScalarAndBmulPerformance)
{
    constexpr uint64_t a = 0x50463C32281E140AULL;
    constexpr uint64_t b = 0xC8000A141E283250ULL;
    constexpr uint8_t t = 128;
    const auto scalar = runInterpolation(a, b, t, false);
    const auto simd = runInterpolation(a, b, t, true);

    ASSERT_EQ(scalar.result, scalarReference(a, b, t));
    ASSERT_EQ(simd.result, scalar.result);
    ASSERT_GT(scalar.instructions, 0);
    ASSERT_GT(simd.instructions, 0);
    ASSERT_GT(scalar.cycles, 0);
    ASSERT_GT(simd.cycles, 0);

    const double instructionReduction =
        (1.0 - static_cast<double>(simd.instructions) / scalar.instructions) * 100.0;
    const double speedup =
        static_cast<double>(scalar.cycles) / simd.cycles;

    std::cout << "\n--- Linear Interpolation Performance (t = 128/255) ---\n"
              << "Scalar cycles: " << scalar.cycles << "\n"
              << "SIMD cycles: " << simd.cycles << "\n"
              << "Instruction reduction: " << instructionReduction << "%\n"
              << "Simulated speedup: " << speedup << "x\n";
}
