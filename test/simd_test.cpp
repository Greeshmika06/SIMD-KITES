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

void expectScalarBadd(uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"

        // Byte 0
        "andi x4, x1, 255\n"
        "andi x5, x2, 255\n"
        "add x6, x4, x5\n"
        "andi x6, x6, 255\n"

        // Byte 1
        "srli x4, x1, 8\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 8\n"
        "andi x5, x5, 255\n"
        "add x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 8\n"
        "or x6, x6, x7\n"

        // Byte 2
        "srli x4, x1, 16\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 16\n"
        "andi x5, x5, 255\n"
        "add x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 16\n"
        "or x6, x6, x7\n"

        // Byte 3
        "srli x4, x1, 24\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 24\n"
        "andi x5, x5, 255\n"
        "add x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 24\n"
        "or x6, x6, x7\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(6), expected);
}

void expectScalarBsub(uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"

        // Byte 0
        "andi x4, x1, 255\n"
        "andi x5, x2, 255\n"
        "sub x6, x4, x5\n"
        "andi x6, x6, 255\n"

        // Byte 1
        "srli x4, x1, 8\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 8\n"
        "andi x5, x5, 255\n"
        "sub x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 8\n"
        "or x6, x6, x7\n"

        // Byte 2
        "srli x4, x1, 16\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 16\n"
        "andi x5, x5, 255\n"
        "sub x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 16\n"
        "or x6, x6, x7\n"

        // Byte 3
        "srli x4, x1, 24\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 24\n"
        "andi x5, x5, 255\n"
        "sub x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 24\n"
        "or x6, x6, x7\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(6), expected);
}

std::unique_ptr<Kites::RVSSProcessor> expectScalarBmin(
    uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"

        // Byte 0
        "andi x4, x1, 255\n"
        "andi x5, x2, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x4, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x5, x10\n"
        "or x6, x9, x10\n"

        // Byte 1
        "srli x4, x1, 8\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 8\n"
        "andi x5, x5, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x4, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x5, x10\n"
        "or x7, x9, x10\n"
        "slli x7, x7, 8\n"
        "or x6, x6, x7\n"

        // Byte 2
        "srli x4, x1, 16\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 16\n"
        "andi x5, x5, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x4, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x5, x10\n"
        "or x7, x9, x10\n"
        "slli x7, x7, 16\n"
        "or x6, x6, x7\n"

        // Byte 3
        "srli x4, x1, 24\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 24\n"
        "andi x5, x5, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x4, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x5, x10\n"
        "or x7, x9, x10\n"
        "slli x7, x7, 24\n"
        "or x6, x6, x7\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(6), expected);
    return vm;
}

std::unique_ptr<Kites::RVSSProcessor> expectScalarBmax(
    uint32_t a, uint32_t b, uint32_t expected)
{
    std::string source =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"

        // Byte 0
        "andi x4, x1, 255\n"
        "andi x5, x2, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x5, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x4, x10\n"
        "or x6, x9, x10\n"

        // Byte 1
        "srli x4, x1, 8\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 8\n"
        "andi x5, x5, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x5, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x4, x10\n"
        "or x7, x9, x10\n"
        "slli x7, x7, 8\n"
        "or x6, x6, x7\n"

        // Byte 2
        "srli x4, x1, 16\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 16\n"
        "andi x5, x5, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x5, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x4, x10\n"
        "or x7, x9, x10\n"
        "slli x7, x7, 16\n"
        "or x6, x6, x7\n"

        // Byte 3
        "srli x4, x1, 24\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 24\n"
        "andi x5, x5, 255\n"
        "sltu x8, x4, x5\n"
        "sub x8, x0, x8\n"
        "and x9, x5, x8\n"
        "xori x10, x8, -1\n"
        "and x10, x4, x10\n"
        "or x7, x9, x10\n"
        "slli x7, x7, 24\n"
        "or x6, x6, x7\n";

    auto vm = runSource(source);

    EXPECT_EQ(vm->registers_.ReadGpr(6), expected);
    return vm;
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



TEST(PackedSimd, ScalarBaddCheck)
{
    expectScalarBadd(0x04030201, 0x08070605, 0x0C0A0806);
}

TEST(PackedSimd, ScalarBsubCheck)
{
    expectScalarBsub(0x08070605, 0x04030201, 0x04040404);
    expectScalarBsub(0x00000000, 0x01010101, 0xFFFFFFFF);
    expectScalarBsub(0x01000100, 0x00010001, 0x01FF01FF);
}

TEST(PackedSimd, ScalarBminCheck)
{
    expectScalarBmin(0x08070605, 0x04030201, 0x04030201);
    expectScalarBmin(0x01020304, 0x05060708, 0x01020304);
    expectScalarBmin(0xFF00FF00, 0x01010101, 0x01000100);
    expectScalarBmin(0xAABBCCDD, 0xAABBCCDD, 0xAABBCCDD);
}

TEST(PackedSimd, ScalarBmaxCheck)
{
    expectScalarBmax(0x08070605, 0x04030201, 0x08070605);
    expectScalarBmax(0x01020304, 0x05060708, 0x05060708);
    expectScalarBmax(0xFF00FF00, 0x01010101, 0xFF01FF01);
    expectScalarBmax(0xAABBCCDD, 0xAABBCCDD, 0xAABBCCDD);
}

TEST(PackedSimd, BaddPerformance)
{
    uint32_t a = 0x04030201;
    uint32_t b = 0x08070605;
    uint32_t expected = 0x0C0A0806;

    // Scalar implementation
    std::string scalarSource =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"

        "andi x4, x1, 255\n"
        "andi x5, x2, 255\n"
        "add x6, x4, x5\n"
        "andi x6, x6, 255\n"

        "srli x4, x1, 8\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 8\n"
        "andi x5, x5, 255\n"
        "add x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 8\n"
        "or x6, x6, x7\n"

        "srli x4, x1, 16\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 16\n"
        "andi x5, x5, 255\n"
        "add x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 16\n"
        "or x6, x6, x7\n"

        "srli x4, x1, 24\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 24\n"
        "andi x5, x5, 255\n"
        "add x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 24\n"
        "or x6, x6, x7\n";

    auto scalarVm = runSource(scalarSource);

    // SIMD implementation
    std::string simdSource =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"
        "badd x3, x1, x2\n";

    auto simdVm = runSource(simdSource);

    EXPECT_EQ(scalarVm->registers_.ReadGpr(6), expected);
    EXPECT_EQ(simdVm->registers_.ReadGpr(3), expected);

    EXPECT_EQ(scalarVm->instructions_retired_, scalarVm->cycle_s_);
EXPECT_EQ(simdVm->instructions_retired_, simdVm->cycle_s_);

    std::cout << "\n--- BADD Performance ---\n";

    std::cout << "Scalar instructions: "
              << scalarVm->instructions_retired_ << "\n";

    std::cout << "Scalar cycles: "
              << scalarVm->cycle_s_ << "\n";

    std::cout << "SIMD instructions: "
              << simdVm->instructions_retired_ << "\n";

    std::cout << "SIMD cycles: "
              << simdVm->cycle_s_ << "\n";

    std::cout << "Instruction reduction: "
              << (1.0 - (double)simdVm->instructions_retired_ /
                            scalarVm->instructions_retired_) * 100
              << "%\n";
        
std::cout << "Simulated speedup: "
          << (double)scalarVm->cycle_s_ / simdVm->cycle_s_
          << "x\n";
}

TEST(PackedSimd, BsubPerformance)
{
    uint32_t a = 0x08070605;
    uint32_t b = 0x04030201;
    uint32_t expected = 0x04040404;

    // Scalar implementation
    std::string scalarSource =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"

        // Byte 0
        "andi x4, x1, 255\n"
        "andi x5, x2, 255\n"
        "sub x6, x4, x5\n"
        "andi x6, x6, 255\n"

        // Byte 1
        "srli x4, x1, 8\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 8\n"
        "andi x5, x5, 255\n"
        "sub x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 8\n"
        "or x6, x6, x7\n"

        // Byte 2
        "srli x4, x1, 16\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 16\n"
        "andi x5, x5, 255\n"
        "sub x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 16\n"
        "or x6, x6, x7\n"

        // Byte 3
        "srli x4, x1, 24\n"
        "andi x4, x4, 255\n"
        "srli x5, x2, 24\n"
        "andi x5, x5, 255\n"
        "sub x7, x4, x5\n"
        "andi x7, x7, 255\n"
        "slli x7, x7, 24\n"
        "or x6, x6, x7\n";

    auto scalarVm = runSource(scalarSource);

    // SIMD implementation
    std::string simdSource =
        "li x1, " + std::to_string(a) + "\n" +
        "li x2, " + std::to_string(b) + "\n"
        "bsub x3, x1, x2\n";

    auto simdVm = runSource(simdSource);

    EXPECT_EQ(scalarVm->registers_.ReadGpr(6), expected);
    EXPECT_EQ(simdVm->registers_.ReadGpr(3), expected);

    EXPECT_EQ(scalarVm->instructions_retired_, scalarVm->cycle_s_);
EXPECT_EQ(simdVm->instructions_retired_, simdVm->cycle_s_);

    std::cout << "\n--- BSUB Performance ---\n";

    std::cout << "Scalar instructions: "
              << scalarVm->instructions_retired_ << "\n";

    std::cout << "Scalar cycles: "
              << scalarVm->cycle_s_ << "\n";

    std::cout << "SIMD instructions: "
              << simdVm->instructions_retired_ << "\n";

    std::cout << "SIMD cycles: "
              << simdVm->cycle_s_ << "\n";

    std::cout << "Instruction reduction: "
              << (1.0 - (double)simdVm->instructions_retired_ /
                            scalarVm->instructions_retired_) * 100
              << "%\n";

    std::cout << "Simulated speedup: "
              << (double)scalarVm->cycle_s_ / simdVm->cycle_s_
              << "x\n";
}

TEST(PackedSimd, BminPerformance)
{
    uint32_t a = 0x08070605;
    uint32_t b = 0x04030201;
    uint32_t expected = 0x04030201;

    // Scalar implementation
    auto scalarVm = expectScalarBmin(a, b, expected);

    // SIMD implementation
    auto simdVm = runSource(
        "li x1, 0x08070605\n"
        "li x2, 0x04030201\n"
        "bmin x3, x1, x2\n"
    );

    EXPECT_EQ(simdVm->registers_.ReadGpr(3), expected);

    EXPECT_EQ(scalarVm->instructions_retired_, scalarVm->cycle_s_);
    EXPECT_EQ(simdVm->instructions_retired_, simdVm->cycle_s_);

    double reduction =
        (1.0 - (double)simdVm->instructions_retired_ /
        scalarVm->instructions_retired_) * 100.0;

    double speedup =
        (double)scalarVm->cycle_s_ / simdVm->cycle_s_;

    std::cout << "\n--- BMIN Performance ---\n";
    std::cout << "Scalar instructions: "
              << scalarVm->instructions_retired_ << std::endl;
    std::cout << "Scalar cycles: "
              << scalarVm->cycle_s_ << std::endl;
    std::cout << "SIMD instructions: "
              << simdVm->instructions_retired_ << std::endl;
    std::cout << "SIMD cycles: "
              << simdVm->cycle_s_ << std::endl;
    std::cout << "Instruction reduction: "
              << reduction << "%" << std::endl;
    std::cout << "Simulated speedup: "
              << speedup << "x" << std::endl;
}


TEST(PackedSimd, BmaxPerformance)
{
    uint32_t a = 0x08070605;
    uint32_t b = 0x04030201;
    uint32_t expected = 0x08070605;

    // Scalar implementation
    auto scalarVm = expectScalarBmax(a, b, expected);

    // SIMD implementation
    auto simdVm = runSource(
        "li x1, 0x08070605\n"
        "li x2, 0x04030201\n"
        "bmax x3, x1, x2\n"
    );

    EXPECT_EQ(simdVm->registers_.ReadGpr(3), expected);

    EXPECT_EQ(scalarVm->instructions_retired_, scalarVm->cycle_s_);
    EXPECT_EQ(simdVm->instructions_retired_, simdVm->cycle_s_);

    double reduction =
        (1.0 - (double)simdVm->instructions_retired_ /
        scalarVm->instructions_retired_) * 100.0;

    double speedup =
        (double)scalarVm->cycle_s_ / simdVm->cycle_s_;

    std::cout << "\n--- BMAX Performance ---\n";
    std::cout << "Scalar instructions: "
              << scalarVm->instructions_retired_ << std::endl;
    std::cout << "Scalar cycles: "
              << scalarVm->cycle_s_ << std::endl;
    std::cout << "SIMD instructions: "
              << simdVm->instructions_retired_ << std::endl;
    std::cout << "SIMD cycles: "
              << simdVm->cycle_s_ << std::endl;
    std::cout << "Instruction reduction: "
              << reduction << "%" << std::endl;
    std::cout << "Simulated speedup: "
              << speedup << "x" << std::endl;
}