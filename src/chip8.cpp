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

chip8_t* init(chip8_t* chip8) {
    if (chip8 == nullptr)
        return nullptr;

    std::memset(chip8->memory, 0, 4096);
    std::memset(chip8->registers, 0, 16);
    std::memset(chip8->stack, 0, 16);

    chip8->pc = PROGRAM_START_ADDRESS;
    chip8->sp = 0;

    chip8->opcode = 0;
    chip8->index = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    chip8->rand_gen(static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()))

    load_fonset(chip8->memory);

    return chip8;
}

chip8_t* make_chip8() {
    auto *chip8 = static_cast<chip8_t *>(std::malloc(sizeof(chip8_t)));
    init(chip8);
    return chip8;
}


Chip8::Chip8()
        : randGen(static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()))
{
    // Initialize PC
    pc = PROGRAM_START_ADDRESS;

    load_fonset(memory);
    // Initialize RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    // Set up function pointer dispatch_table
    dispatch_table[0x0] = &Chip8::Table0;
    dispatch_table[0x1] = &Chip8::OP_1nnn;
    dispatch_table[0x2] = &Chip8::OP_2nnn;
    dispatch_table[0x3] = &Chip8::OP_3xkk;
    dispatch_table[0x4] = &Chip8::OP_4xkk;
    dispatch_table[0x5] = &Chip8::OP_5xy0;
    dispatch_table[0x6] = &Chip8::OP_6xkk;
    dispatch_table[0x7] = &Chip8::OP_7xkk;
    dispatch_table[0x8] = &Chip8::Table8;
    dispatch_table[0x9] = &Chip8::OP_9xy0;
    dispatch_table[0xA] = &Chip8::OP_Annn;
    dispatch_table[0xB] = &Chip8::OP_Bnnn;
    dispatch_table[0xC] = &Chip8::OP_Cxkk;
    dispatch_table[0xD] = &Chip8::OP_Dxyn;
    dispatch_table[0xE] = &Chip8::TableE;
    dispatch_table[0xF] = &Chip8::TableF;

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
    ((*this).*(dispatch_table[(opcode & 0xF000u) >> 12u]))();

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
    std::memset(this->video, 0, sizeof(this->video));
}

void Chip8::OP_00EE()
{
    this->sp -= 1;
    pc = this->stack[sp];
}

void Chip8::OP_1nnn()
{
    uint16_t nnn = make_nnn(opcode);

    this->pc = nnn;
}

void Chip8::OP_2nnn()
{
    uint16_t nnn = make_nnn(opcode);

    this->stack[this->sp] = this->pc;
    this->sp += 1;
    this->pc = nnn;
}

void Chip8::OP_3xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t kk = make_kk(opcode);

    if (this->registers[vx] == kk)
    {
        this->pc += 2;
    }
}

void Chip8::OP_4xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t kk = make_kk(opcode);

    if (this->registers[vx] != kk)
    {
        this->pc += 2;
    }
}

void Chip8::OP_5xy0()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (this->registers[vx] == this->registers[vy])
    {
        this->pc += 2;
    }
}

void Chip8::OP_6xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t kk = make_kk(opcode);

    this->registers[vx] = kk;
}

void Chip8::OP_7xkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t kk = make_kk(opcode);

    this->registers[vx] += kk;
}

void Chip8::OP_8xy0()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    this->registers[vx] = this->registers[vy];
}

void Chip8::OP_8xy1()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    this->registers[vx] |= this->registers[vy];
}

void Chip8::OP_8xy2()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    this->registers[vx] &= this->registers[vy];
}

void Chip8::OP_8xy3()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    this->registers[vx] ^= this->registers[vy];
}

void Chip8::OP_8xy4()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    uint16_t sum = this->registers[vx] + this->registers[vy];

    if (sum > 255U)
    {
        this->registers[0xF] = 1;
    }
    else
    {
        this->registers[0xF] = 0;
    }

    this->registers[vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (this->registers[vx] > this->registers[vy])
    {
        this->registers[0xF] = 1;
    }
    else
    {
        this->registers[0xF] = 0;
    }

    this->registers[vx] -= this->registers[vy];
}

void Chip8::OP_8xy6()
{
    uint8_t vx = make_vx(opcode);

    // Save LSB in VF
    this->registers[0xF] = (this->registers[vx] & 0x1u);

    this->registers[vx] >>= 1;
}

void Chip8::OP_8xy7()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (this->registers[vy] > this->registers[vx])
    {
        this->registers[0xF] = 1;
    }
    else
    {
        this->registers[0xF] = 0;
    }

    this->registers[vx] = this->registers[vy] - this->registers[vx];
}

void Chip8::OP_8xyE()
{
    uint8_t vx = make_vx(opcode);

    // Save MSB in VF
    this->registers[0xF] = (this->registers[vx] & 0x80u) >> 7u;

    this->registers[vx] <<= 1;
}

void Chip8::OP_9xy0()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);

    if (this->registers[vx] != this->registers[vy])
    {
        pc += 2;
    }
}

void Chip8::OP_Annn()
{
    uint16_t nnn = make_nnn(opcode);

    index = nnn;
}

void Chip8::OP_Bnnn()
{
    uint16_t nnn = make_nnn(opcode);

    pc = this->registers[0] + nnn;
}

void Chip8::OP_Cxkk()
{
    uint8_t vx = make_vx(opcode);
    uint8_t kk = make_kk(opcode);

    this->registers[vx] = randByte(randGen) & kk;
}

void Chip8::OP_Dxyn()
{
    uint8_t vx = make_vx(opcode);
    uint8_t vy = make_vy(opcode);
    uint8_t height = opcode & 0x000Fu;

    // Wrap if going beyond screen boundaries
    uint8_t xPos = this->registers[vx] % VIDEO_WIDTH;
    uint8_t yPos = this->registers[vy] % VIDEO_HEIGHT;

    this->registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        uint8_t spriteByte = this->memory[index + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t* screenPixel = &this->video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel)
            {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF)
                {
                    this->registers[0xF] = 1;
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

    uint8_t key = this->registers[vx];

    if (this->keypad[key])
    {
        this->pc += 2;
    }
}

void Chip8::OP_ExA1()
{
    uint8_t vx = make_vx(opcode);

    uint8_t key = this->registers[vx];

    if (!this->keypad[key])
    {
        this->pc += 2;
    }
}

void Chip8::OP_Fx07()
{
    uint8_t vx = make_vx(opcode);

    this->registers[vx] = delayTimer;
}

void Chip8::OP_Fx0A()
{
    uint8_t vx = make_vx(opcode);

    if (this->keypad[0])
    {
        this->registers[vx] = 0;
    }
    else if (this->keypad[1])
    {
        this->registers[vx] = 1;
    }
    else if (this->keypad[2])
    {
        this->registers[vx] = 2;
    }
    else if (this->keypad[3])
    {
        this->registers[vx] = 3;
    }
    else if (this->keypad[4])
    {
        this->registers[vx] = 4;
    }
    else if (this->keypad[5])
    {
        this->registers[vx] = 5;
    }
    else if (this->keypad[6])
    {
        this->registers[vx] = 6;
    }
    else if (this->keypad[7])
    {
        this->registers[vx] = 7;
    }
    else if (this->keypad[8])
    {
        this->registers[vx] = 8;
    }
    else if (this->keypad[9])
    {
        this->registers[vx] = 9;
    }
    else if (this->keypad[10])
    {
        this->registers[vx] = 10;
    }
    else if (this->keypad[11])
    {
        this->registers[vx] = 11;
    }
    else if (this->keypad[12])
    {
        this->registers[vx] = 12;
    }
    else if (this->keypad[13])
    {
        this->registers[vx] = 13;
    }
    else if (this->keypad[14])
    {
        this->registers[vx] = 14;
    }
    else if (this->keypad[15])
    {
        this->registers[vx] = 15;
    }
    else
    {
        this->pc -= 2;
    }
}

void Chip8::OP_Fx15()
{
    uint8_t vx = make_vx(opcode);

    this->delayTimer = this->registers[vx];
}

void Chip8::OP_Fx18()
{
    uint8_t vx = make_vx(opcode);

    this->soundTimer = this->registers[vx];
}

void Chip8::OP_Fx1E()
{
    uint8_t vx = make_vx(opcode);

    this->index += this->registers[vx];
}

void Chip8::OP_Fx29()
{
    uint8_t vx = make_vx(opcode);
    uint8_t digit = this->registers[vx];

    this->index = static_cast<uint16_t>(FONTSET_START_ADDRESS + (5 * digit));
}

void Chip8::OP_Fx33()
{
    uint8_t vx = make_vx(opcode);
    uint8_t value = this->registers[vx];

    // Ones-place
    this->memory[this->index + 2] = value % 10;
    value /= 10;

    // Tens-place
    this->memory[this->index + 1] = value % 10;
    value /= 10;

    // Hundreds-place
    this->memory[this->index] = value % 10;
}

void Chip8::OP_Fx55()
{
    uint8_t vx = make_vx(opcode);

    for (uint8_t i = 0; i <= vx; ++i)
    {
        this->memory[this->index + i] = this->registers[i];
    }
}

void Chip8::OP_Fx65()
{
    uint8_t vx = make_vx(opcode);

    for (uint8_t i = 0; i <= vx; ++i)
    {
        this->registers[i] = this->memory[this->index + i];
    }
}