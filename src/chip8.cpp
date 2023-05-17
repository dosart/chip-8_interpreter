#include "chip8.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <random>

#include <iostream>
#include <vector>
#include <filesystem>



const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

const unsigned int PROGRAM_START_ADDRESS = 0x200;

const unsigned int MAX_MEMORY = 0xFFF;
const int MAX_ROM_SIZE = MAX_MEMORY - 0x200;

using bytes_t = std::vector<std::byte>;
using path_t = std::filesystem::path;
using size_t = std::vector<int>::size_type;


void load_fonset(uint8_t* memory){
    uint8_t chip8_fontset[FONTSET_SIZE] =
            {
                    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                    0x20, 0x60, 0x20, 0x20, 0x70, // 1
                    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
            };

    std::memcpy(memory + FONTSET_START_ADDRESS, chip8_fontset, FONTSET_SIZE);
}


Chip8::Chip8()
        : randGen(static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()))
{
    // Initialize PC
    pc = PROGRAM_START_ADDRESS;

    load_fonset(memory);
    // Initialize RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    // Set up function pointer table
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    for (size_t i = 0; i <= 0xE; i++)
    {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    for (size_t i = 0; i <= 0x65; i++)
    {
        tableF[i] = &Chip8::OP_NULL;
    }

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

bytes_t read_program(path_t filepath)
{
    std::ifstream file(filepath, std::ios::out | std::ios::binary);
    if (!file.is_open())
        return bytes_t();

    file.seekg(0, std::ios_base::end);
    auto length = file.tellg();
    file.seekg(0, std::ios_base::beg);

    bytes_t buffer(static_cast<size_t>(length));
    file.read(reinterpret_cast<char*>(buffer.data()), length);

    file.close();
    return buffer;
}

void load_program(uint8_t* memory, const bytes_t& program){
    std::memcpy(memory + PROGRAM_START_ADDRESS, program.data(), program.size());
}

void Chip8::LoadROM(char const* filename)
{
    auto program = read_program(filename);
    if(program.size() > MAX_ROM_SIZE)
        throw std::runtime_error("File size is bigger than max rom size.");
    else if (program.size() <= 0)
        throw std::runtime_error("No file or empty file.");

    load_program(memory, program);
}

void Chip8::Cycle()
{
    // Fetch
    opcode = static_cast<uint16_t>((memory[pc] << 8u) | memory[pc + 1]);

    // Increment the PC before we execute anything
    pc += 2;

    // Decode and Execute
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    // Decrement the delay timer if it's been set
    if (delayTimer > 0)
    {
        --delayTimer;
    }

    // Decrement the sound timer if it's been set
    if (soundTimer > 0)
    {
        --soundTimer;
    }
}

void Chip8::Table0()
{
    ((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8()
{
    ((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
    ((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF()
{
    ((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL()
{}

uint8_t make_vx(uint16_t opcode) {
    return ((opcode & 0x0F00u) >> 8u);
}

uint8_t make_vy(uint16_t opcode) {
    return ((opcode & 0x00F0u) >> 4u);
}

uint8_t make_kk(uint16_t opcode){
    return (opcode & 0x00FFu);
}

uint16_t make_nnn(uint16_t opcode){
    return (opcode & 0x0FFFu);
}


void Chip8::OP_00E0()
{
    memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE()
{
    --sp;
    pc = stack[sp];
}

void Chip8::OP_1nnn()
{
    uint16_t address = make_nnn(opcode);

    pc = address;
}

void Chip8::OP_2nnn()
{
    uint16_t address = make_nnn(opcode);

    stack[sp] = pc;
    ++sp;
    pc = address;
}

void Chip8::OP_3xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t byte = make_kk(opcode);

    if (registers[vx] == byte)
    {
        pc += 2;
    }
}

void Chip8::OP_4xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t byte = make_kk(opcode);

    if (registers[vx] != byte)
    {
        pc += 2;
    }
}

void Chip8::OP_5xy0()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (registers[vx] == registers[vy])
    {
        pc += 2;
    }
}

void Chip8::OP_6xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t byte = make_kk(opcode);

    registers[vx] = byte;
}

void Chip8::OP_7xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t byte = make_kk(opcode);

    registers[vx] += byte;
}

void Chip8::OP_8xy0()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    registers[vx] = registers[vy];
}

void Chip8::OP_8xy1()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    registers[vx] |= registers[vy];
}

void Chip8::OP_8xy2()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    registers[vx] &= registers[vy];
}

void Chip8::OP_8xy3()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    registers[vx] ^= registers[vy];
}

void Chip8::OP_8xy4()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    uint16_t sum = registers[vx] + registers[vy];

    if (sum > 255U)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (registers[vx] > registers[vy])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[vx] -= registers[vy];
}

void Chip8::OP_8xy6()
{
    uint8_t vx = make_vx(opcode);

    // Save LSB in VF
    registers[0xF] = (registers[vx] & 0x1u);

    registers[vx] >>= 1;
}

void Chip8::OP_8xy7()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (registers[vy] > registers[vx])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[vx] = registers[vy] - registers[vx];
}

void Chip8::OP_8xyE()
{
    uint8_t vx = make_vx(opcode);

    // Save MSB in VF
    registers[0xF] = (registers[vx] & 0x80u) >> 7u;

    registers[vx] <<= 1;
}

void Chip8::OP_9xy0()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (registers[vx] != registers[vy])
    {
        pc += 2;
    }
}

void Chip8::OP_Annn()
{
    uint16_t address = make_nnn(opcode);

    index = address;
}

void Chip8::OP_Bnnn()
{
    uint16_t address = make_nnn(opcode);

    pc = registers[0] + address;
}

void Chip8::OP_Cxkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t byte = make_kk(opcode);

    registers[vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);
    uint8_t height = opcode & 0x000Fu;

    // Wrap if going beyond screen boundaries
    uint8_t xPos = registers[vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel)
            {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF)
                {
                    registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E()
{
    uint8_t vx = make_vx(opcode);

    uint8_t key = registers[vx];

    if (keypad[key])
    {
        pc += 2;
    }
}

void Chip8::OP_ExA1()
{
    uint8_t vx = make_vx(opcode);

    uint8_t key = registers[vx];

    if (!keypad[key])
    {
        pc += 2;
    }
}

void Chip8::OP_Fx07()
{
    uint8_t vx = make_vx(opcode);

    registers[vx] = delayTimer;
}

void Chip8::OP_Fx0A()
{
    uint8_t vx = make_vx(opcode);

    if (keypad[0])
    {
        registers[vx] = 0;
    }
    else if (keypad[1])
    {
        registers[vx] = 1;
    }
    else if (keypad[2])
    {
        registers[vx] = 2;
    }
    else if (keypad[3])
    {
        registers[vx] = 3;
    }
    else if (keypad[4])
    {
        registers[vx] = 4;
    }
    else if (keypad[5])
    {
        registers[vx] = 5;
    }
    else if (keypad[6])
    {
        registers[vx] = 6;
    }
    else if (keypad[7])
    {
        registers[vx] = 7;
    }
    else if (keypad[8])
    {
        registers[vx] = 8;
    }
    else if (keypad[9])
    {
        registers[vx] = 9;
    }
    else if (keypad[10])
    {
        registers[vx] = 10;
    }
    else if (keypad[11])
    {
        registers[vx] = 11;
    }
    else if (keypad[12])
    {
        registers[vx] = 12;
    }
    else if (keypad[13])
    {
        registers[vx] = 13;
    }
    else if (keypad[14])
    {
        registers[vx] = 14;
    }
    else if (keypad[15])
    {
        registers[vx] = 15;
    }
    else
    {
        pc -= 2;
    }
}

void Chip8::OP_Fx15()
{
    uint8_t vx = make_vx(opcode);

    delayTimer = registers[vx];
}

void Chip8::OP_Fx18()
{
    uint8_t vx = make_vx(opcode);

    soundTimer = registers[vx];
}

void Chip8::OP_Fx1E()
{
    uint8_t vx = make_vx(opcode);

    index += registers[vx];
}

void Chip8::OP_Fx29()
{
    uint8_t vx = make_vx(opcode);
    uint8_t digit = registers[vx];

    index = static_cast<uint16_t>(FONTSET_START_ADDRESS + (5 * digit));
}

void Chip8::OP_Fx33()
{
    uint8_t vx = make_vx(opcode);
    uint8_t value = registers[vx];

    // Ones-place
    memory[index + 2] = value % 10;
    value /= 10;

    // Tens-place
    memory[index + 1] = value % 10;
    value /= 10;

    // Hundreds-place
    memory[index] = value % 10;
}

void Chip8::OP_Fx55()
{
    uint8_t vx = make_vx(opcode);

    for (uint8_t i = 0; i <= vx; ++i)
    {
        memory[index + i] = registers[i];
    }
}

void Chip8::OP_Fx65()
{
    uint8_t vx = make_vx(opcode);

    for (uint8_t i = 0; i <= vx; ++i)
    {
        registers[i] = memory[index + i];
    }
}