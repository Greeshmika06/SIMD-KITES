#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"
#include "utils/utils.h"

namespace
{

struct ProcessingResult
{
    std::vector<uint64_t> output;
    uint64_t instructions;
    uint64_t cycles;
};

std::vector<uint64_t> loadPackedImage()
{
    std::ifstream input("image_processing/input.txt");

    if (!input)
        throw std::runtime_error("Could not open input.txt");

    std::vector<uint64_t> pixels;
    std::string line;

    while (std::getline(input, line))
    {
        if (!line.empty())
            pixels.push_back(std::stoull(line, nullptr, 16));
    }

    return pixels;
}

void loadImageIntoMemory(
    Kites::RVSSProcessor &vm,
    const std::vector<uint64_t> &pixels)
{
    const uint64_t INPUT_BASE = 0x10000000;

    for (size_t i = 0; i < pixels.size(); i++)
    {
        vm.memory_controller_.writeDoubleWord_d(
            INPUT_BASE + i * 8,
            pixels[i]
        );
    }
}

// Run either SIMD or Scalar brightness processing
ProcessingResult runBrightness(
    const std::vector<uint64_t> &pixels,
    bool useSimd,
    const std::string &outputFile)
{
    Kites::setupVmStateDirectory();

    auto vm = std::make_unique<Kites::RVSSProcessor>();

    std::string source;

    if (useSimd)
    {
        source =
            "li x10, 0x10000000\n"
            "li x11, 0x10020000\n"
            "li x12, " + std::to_string(pixels.size()) + "\n"
            "li x2, 0x2828282828282828\n"

            "loop:\n"
            "ld x1, 0(x10)\n"
            "badds x3, x1, x2\n"
            "sd x3, 0(x11)\n"

            "addi x10, x10, 8\n"
            "addi x11, x11, 8\n"
            "addi x12, x12, -1\n"
            "bne x12, x0, loop\n";
    }
    else
    {
        source =
            "li x10, 0x10000000\n"
            "li x11, 0x10020000\n"
            "li x12, " + std::to_string(pixels.size()) + "\n"

            "loop:\n"
            "ld x1, 0(x10)\n"

            // Byte 0
            "andi x3, x1, 255\n"
            "addi x3, x3, 40\n"
            "sltiu x5, x3, 256\n"
            "sub x5, x0, x5\n"
            "and x3, x3, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x3, x3, x6\n"

            // Byte 1
            "srli x4, x1, 8\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 8\n"
            "or x3, x3, x4\n"

            // Byte 2
            "srli x4, x1, 16\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 16\n"
            "or x3, x3, x4\n"

            // Byte 3
            "srli x4, x1, 24\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 24\n"
            "or x3, x3, x4\n"

            // Byte 4
            "srli x4, x1, 32\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 32\n"
            "or x3, x3, x4\n"

            // Byte 5
            "srli x4, x1, 40\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 40\n"
            "or x3, x3, x4\n"

            // Byte 6
            "srli x4, x1, 48\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 48\n"
            "or x3, x3, x4\n"

            // Byte 7
            "srli x4, x1, 56\n"
            "andi x4, x4, 255\n"
            "addi x4, x4, 40\n"
            "sltiu x5, x4, 256\n"
            "sub x5, x0, x5\n"
            "and x4, x4, x5\n"
            "xori x6, x5, -1\n"
            "andi x6, x6, 255\n"
            "or x4, x4, x6\n"
            "slli x4, x4, 56\n"
            "or x3, x3, x4\n"

            "sd x3, 0(x11)\n"

            "addi x10, x10, 8\n"
            "addi x11, x11, 8\n"
            "addi x12, x12, -1\n"
            "bne x12, x0, loop\n";
    }

    std::istringstream stream(source);
    Kites::AssembledProgram program = Kites::assemble(stream);

    vm->LoadProgram(program);

    loadImageIntoMemory(*vm, pixels);

    vm->RunFast();

    ProcessingResult result;

    result.instructions = vm->instructions_retired_;
    result.cycles = vm->cycle_s_;

    const uint64_t OUTPUT_BASE = 0x10020000;

    std::ofstream output(outputFile);

    if (!output.is_open())
        throw std::runtime_error("Could not open output file: " + outputFile);

    for (size_t i = 0; i < pixels.size(); i++)
    {
        uint64_t value =
            vm->memory_controller_.readDoubleWord_d(
                OUTPUT_BASE + i * 8
            );

        result.output.push_back(value);

        output << std::hex << std::uppercase
               << std::setw(16) << std::setfill('0')
               << value << "\n";
    }

    return result;
}

// Compare SIMD and Scalar outputs
void compareOutputs(
    const std::vector<uint64_t> &simdOutput,
    const std::vector<uint64_t> &scalarOutput)
{
    ASSERT_EQ(simdOutput.size(), scalarOutput.size());

    size_t mismatches = 0;

    for (size_t i = 0; i < simdOutput.size(); i++)
    {
        if (simdOutput[i] != scalarOutput[i])
        {
            mismatches++;

            if (mismatches <= 5)
            {
                std::cout << "Mismatch at packed value " << i
                          << ": SIMD = 0x" << std::hex << simdOutput[i]
                          << ", Scalar = 0x" << scalarOutput[i]
                          << std::dec << "\n";
            }
        }
    }

    std::cout << "\n========== OUTPUT COMPARISON ==========\n";

    std::cout << "Total packed values: "
              << simdOutput.size() << "\n";

    std::cout << "Total pixels: "
              << simdOutput.size() * 8 << "\n";

    std::cout << "Mismatches: " << mismatches << "\n";

    if (mismatches == 0)
        std::cout << "RESULT: SIMD and Scalar outputs MATCH!\n";
    else
        std::cout << "RESULT: OUTPUTS DO NOT MATCH!\n";

    EXPECT_EQ(mismatches, 0);
}

// Performance evaluation
void evaluatePerformance(
    uint64_t simdInstructions,
    uint64_t scalarInstructions,
    uint64_t simdCycles,
    uint64_t scalarCycles)
{
    if (scalarInstructions == 0 ||
        scalarCycles == 0 ||
        simdCycles == 0)
    {
        std::cout << "Invalid performance counters.\n";
        return;
    }

    double instructionReduction =
        (1.0 - (double)simdInstructions / scalarInstructions) * 100.0;

    double cycleReduction =
        (1.0 - (double)simdCycles / scalarCycles) * 100.0;

    double speedup =
        (double)scalarCycles / simdCycles;

    std::cout << "\n========== PERFORMANCE EVALUATION ==========\n";

    std::cout << "SIMD instructions: "
              << simdInstructions << "\n";

    std::cout << "Scalar instructions: "
              << scalarInstructions << "\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "Instruction reduction: "
              << instructionReduction << "%\n";

    std::cout << "SIMD cycles: "
              << simdCycles << "\n";

    std::cout << "Scalar cycles: "
              << scalarCycles << "\n";

    std::cout << "Cycle reduction: "
              << cycleReduction << "%\n";

    std::cout << "Simulated speedup: "
              << speedup << "x\n";
}

// Main comparison test
TEST(ImageProcessing, BrightnessComparisonAndPerformance)
{
    auto pixels = loadPackedImage();

    ASSERT_FALSE(pixels.empty());

    std::cout << "\n========== IMAGE PROCESSING ==========\n";
    std::cout << "Packed values: " << pixels.size() << "\n";
    std::cout << "Total pixels: " << pixels.size() * 8 << "\n";

    // Run SIMD implementation
    auto simdResult = runBrightness(
        pixels,
        true,
        "image_processing/simd_output.txt"
    );

    // Run Scalar implementation
    auto scalarResult = runBrightness(
        pixels,
        false,
        "image_processing/scalar_output.txt"
    );

    // Compare outputs
    compareOutputs(
        simdResult.output,
        scalarResult.output
    );

    // Display performance
    evaluatePerformance(
        simdResult.instructions,
        scalarResult.instructions,
        simdResult.cycles,
        scalarResult.cycles
    );

    std::cout << "\nOutput files generated successfully.\n";
}

}
