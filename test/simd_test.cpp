#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>

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
    vm->DebugRun();

    return vm;
}

void expectBadd(uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n" +
        "badd x3, x1, x2\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(3), expected);
}
void expectBsub(uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n" +
        "bsub x3, x1, x2\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(3), expected);
}
void expectBmin(uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n" +
        "bmin x3, x1, x2\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(3), expected);
}

void expectBmax(uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n" +
        "bmax x3, x1, x2\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(3), expected);
}

} 

TEST(PackedSimd, Badd)
{
    // 1. Basic addition
    expectBadd(0x04030201, 0x08070605, 0x0C0A0806);

    // 2. Adding zero
    expectBadd(0x12345678, 0x00000000, 0x12345678);

    // 3. Byte overflow
    expectBadd(0xFFFFFFFF, 0x01010101, 0x00000000);

    // 4. Independent byte lanes
    expectBadd(0xFF00FF00, 0x01010101, 0x00010001);

    // 5. Maximum byte values
    expectBadd(0xFFFFFFFF, 0xFFFFFFFF, 0xFEFEFEFE);
}

TEST(PackedSimd, Bsub)
{
    // 1. Basic subtraction
    expectBsub(0x08070605, 0x04030201, 0x04040404);

    // 2. Subtracting zero
    expectBsub(0x12345678, 0x00000000, 0x12345678);

    // 3. Byte underflow
    expectBsub(0x00000000, 0x01010101, 0xFFFFFFFF);

    // 4. Independent byte lanes (no borrow between lanes)
    expectBsub(0x01000100, 0x00010001, 0x01FF01FF);

    // 5. Equal operands
    expectBsub(0xAABBCCDD, 0xAABBCCDD, 0x00000000);
}

TEST(PackedSimd, Bmin)
{
    // 1. Basic minimum
    expectBmin(0x08070605, 0x04030201, 0x04030201);

    // 2. First operand is smaller
    expectBmin(0x01020304, 0x05060708, 0x01020304);

    // 3. Second operand is smaller
    expectBmin(0x05060708, 0x01020304, 0x01020304);

    // 4. Equal operands
    expectBmin(0xAABBCCDD, 0xAABBCCDD, 0xAABBCCDD);

    // 5. Unsigned comparison (0xFF is greater than 0x01)
    expectBmin(0xFF00FF00, 0x01010101, 0x01000100);
}

TEST(PackedSimd, Bmax)
{
    // 1. Basic maximum
    expectBmax(0x08070605, 0x04030201, 0x08070605);

    // 2. First operand is larger
    expectBmax(0x05060708, 0x01020304, 0x05060708);

    // 3. Second operand is larger
    expectBmax(0x01020304, 0x05060708, 0x05060708);

    // 4. Equal operands
    expectBmax(0xAABBCCDD, 0xAABBCCDD, 0xAABBCCDD);

    // 5. Unsigned comparison (0xFF is greater than 0x01)
    expectBmax(0xFF00FF00, 0x01010101, 0xFF01FF01);
}