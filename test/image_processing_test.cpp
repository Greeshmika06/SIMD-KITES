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

std::unique_ptr<Kites::RVSSProcessor> runSource(const std::string &source)
{
    Kites::setupVmStateDirectory();

    auto vm = std::make_unique<Kites::RVSSProcessor>();

    std::istringstream stream(source);
    Kites::AssembledProgram program = Kites::assemble(stream);

    vm->LoadProgram(program);
    vm->RunFast();

    return vm;
}

std::vector<uint32_t> readPixels()
{
    std::ifstream file("image_processing/input.txt");

    std::vector<uint32_t> pixels;
    std::string line;

    while (std::getline(file, line))
    {
        pixels.push_back(
            static_cast<uint32_t>(std::stoul(line, nullptr, 16))
        );
    }

    return pixels;
}

std::vector<uint32_t> loadPackedImage()
{
    std::ifstream input("image_processing/input.txt");

    if (!input)
        throw std::runtime_error("Could not open input.txt");

    std::vector<uint32_t> pixels;
    std::string line;

    while (std::getline(input, line))
    {
        if (!line.empty())
        {
            pixels.push_back(
                static_cast<uint32_t>(std::stoul(line, nullptr, 16))
            );
        }
    }

    return pixels;
}
void loadImageIntoMemory(
    Kites::RVSSProcessor &vm,
    const std::vector<uint32_t> &pixels)
{
    const uint64_t INPUT_BASE = 0x10000000;

    for (size_t i = 0; i < pixels.size(); i++)
    {
        vm.memory_controller_.writeWord_d(
            INPUT_BASE + i * 4,
            pixels[i]
        );
    }
}

TEST(ImageProcessing, SimdBrightness)
{
    auto pixels = loadPackedImage();

    ASSERT_FALSE(pixels.empty());

    auto vm = std::make_unique<Kites::RVSSProcessor>();

    std::string source =
        "li x10, 0x10000000\n"
        "li x11, 0x10020000\n"
        "li x12, " + std::to_string(pixels.size()) + "\n"
        "li x2, 0x28282828\n"
        "loop:\n"
        "lw x1, 0(x10)\n"
        "badd x3, x1, x2\n"
        "sw x3, 0(x11)\n"
        "addi x10, x10, 4\n"
        "addi x11, x11, 4\n"
        "addi x12, x12, -1\n"
        "bne x12, x0, loop\n";

    std::istringstream stream(source);
    Kites::AssembledProgram program = Kites::assemble(stream);

    vm->LoadProgram(program);

    loadImageIntoMemory(*vm, pixels);

    vm->RunFast();

    std::ofstream output("image_processing/simd_output.txt");
    ASSERT_TRUE(output.is_open());

    const uint64_t OUTPUT_BASE = 0x10020000;

    for (size_t i = 0; i < pixels.size(); i++)
    {
        uint32_t result = vm->memory_controller_.readWord_d(
            OUTPUT_BASE + i * 4
        );

        output << std::hex << std::uppercase
               << std::setw(8) << std::setfill('0')
               << result << "\n";
    }

    std::cout << "\n--- SIMD Brightness Results ---\n";
    std::cout << "Packed values: " << pixels.size() << "\n";
    std::cout << "SIMD instructions: "
              << vm->instructions_retired_ << "\n";
    std::cout << "SIMD cycles: " << vm->cycle_s_ << "\n";
}

TEST(ImageProcessing, ScalarBrightness)
{
    auto pixels = loadPackedImage();

    ASSERT_FALSE(pixels.empty());

    auto vm = std::make_unique<Kites::RVSSProcessor>();

    std::string source =
        "li x10, 0x10000000\n"
        "li x11, 0x10020000\n"
        "li x12, " + std::to_string(pixels.size()) + "\n"

        "loop:\n"
        "lw x1, 0(x10)\n"

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

        "sw x3, 0(x11)\n"
        "addi x10, x10, 4\n"
        "addi x11, x11, 4\n"
        "addi x12, x12, -1\n"
        "bne x12, x0, loop\n";

    std::istringstream stream(source);
    Kites::AssembledProgram program = Kites::assemble(stream);

    vm->LoadProgram(program);
    loadImageIntoMemory(*vm, pixels);
    vm->RunFast();

    std::ofstream output("image_processing/scalar_output.txt");
    ASSERT_TRUE(output.is_open());

    const uint64_t OUTPUT_BASE = 0x10020000;

    for (size_t i = 0; i < pixels.size(); i++)
    {
        uint32_t result = vm->memory_controller_.readWord_d(
            OUTPUT_BASE + i * 4
        );

        output << std::hex << std::uppercase
               << std::setw(8) << std::setfill('0')
               << result << "\n";
    }

    std::cout << "\n--- Scalar Saturating Brightness Results ---\n";
    std::cout << "Packed values: " << pixels.size() << "\n";
    std::cout << "Scalar instructions: "
              << vm->instructions_retired_ << "\n";
    std::cout << "Scalar cycles: " << vm->cycle_s_ << "\n";
}


}