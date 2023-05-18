#include <string>
#include <vector>
#include <memory>

#include <cstdint>
#include <random>
#include <chrono>
#include <cstddef>

#include <fstream>
#include <filesystem>

#include <chrono>
#include <cstring>
#include <fstream>
#include <random>

#include <iostream>
#include <vector>
#include <filesystem>

const unsigned int KEY_COUNT = 16;
const unsigned int REGISTER_COUNT = 16;

const unsigned int MEMORY_SIZE = 4096;
const unsigned int STACK_SIZE = 16;

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

const unsigned int PROGRAM_START_ADDRESS = 0x200;

const unsigned int MAX_MEMORY = 0xFFF;
const int MAX_ROM_SIZE = MAX_MEMORY - 0x200;

using bytes_t = std::vector<std::byte>;
using path_t = std::filesystem::path;
using size_t = std::vector<int>::size_type;

struct chip8_t {

    uint8_t keypad[KEY_COUNT]{};
    uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
    uint8_t memory[MEMORY_SIZE]{};
    uint8_t registers[REGISTER_COUNT]{};
    uint16_t index{};
    uint16_t pc{};
    uint8_t delay_timer{};
    uint8_t sound_timer{};
    uint16_t stack[STACK_SIZE]{};
    uint8_t sp{};
    uint16_t opcode{};
};

using chip8_ptr_t = std::unique_ptr<chip8_t>;
using func_ptr = void (*)(chip8_t *chip8);


void load_fonset(uint8_t* memory) {
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

    std::memset(chip8->memory, 0, sizeof(chip8->memory));
    std::memset(chip8->registers, 0, sizeof(chip8->registers));
    std::memset(chip8->stack, 0, sizeof(chip8->stack));

    chip8->pc = PROGRAM_START_ADDRESS;
    chip8->sp = 0;

    chip8->opcode = 0;
    chip8->index = 0;

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    load_fonset(chip8->memory);
    return chip8;
}

chip8_ptr_t make_chip8() {
    auto chip8 =  std::make_unique<chip8_t>();
    init(chip8.get());

    return chip8;
}

uint8_t make_vx(uint16_t opcode) {
    return ((opcode & 0x0F00u) >> 8u);
}

uint8_t make_vy(uint16_t opcode) {
    return ((opcode & 0x00F0u) >> 4u);
}

uint8_t make_kk(uint16_t opcode) {
    return (opcode & 0x00FFu);
}

uint16_t make_nnn(uint16_t opcode) {
    return (opcode & 0x0FFFu);
}


void op_00E0(chip8_t* chip8)
{
    std::memset(chip8->video, 0, sizeof(chip8->video));
}

void op_00EE(chip8_t* chip8)
{
    chip8->sp -=1;
    chip8->pc = chip8->stack[chip8->sp];
}

void op_1nnn(chip8_t* chip8)
{
    uint16_t nnn = make_nnn(chip8->opcode);
    chip8->pc = nnn;
}

void op_2nnn(chip8_t* chip8)
{
    uint16_t nnn = make_nnn(chip8->opcode);

    chip8->stack[chip8->sp] = chip8->pc;
    chip8->sp += 1;
    chip8->pc = nnn;
}

void op_3xkk(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t kk = make_kk(chip8->opcode);

    if (chip8->registers[vx] == kk)
        chip8->pc += 2;
}

void op_4xkk(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t kk = make_kk(chip8->opcode);

    if (chip8->registers[vx] != kk)
        chip8->pc += 2;
}

void op_5xy0(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    if (chip8->registers[vx] == chip8->registers[vy])
        chip8->pc += 2;
}

void op_6xkk(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t kk = make_kk(chip8->opcode);

    chip8->registers[vx] = kk;
}

void op_7xkk(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t kk = make_kk(chip8->opcode);

    chip8->registers[vx] += kk;
}

void op_8xy0(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    chip8->registers[vx] = chip8->registers[vy];
}

void op_8xy1(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    chip8->registers[vx] |= chip8->registers[vy];
}

void op_8xy2(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    chip8->registers[vx] &= chip8->registers[vy];
}

void op_8xy3(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    chip8->registers[vx] ^= chip8->registers[vy];
}

void op_8xy4(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    uint16_t sum = chip8->registers[vx] + chip8->registers[vy];

    if (sum > 255U)
        chip8->registers[0xF] = 1;
    else
        chip8->registers[0xF] = 0;

    chip8->registers[vx] = sum & 0xFFu;
}

void op_8xy5(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    if (chip8->registers[vx] > chip8->registers[vy])
        chip8->registers[0xF] = 1;
    else
        chip8->registers[0xF] = 0;

    chip8->registers[vx] -= chip8->registers[vy];
}

void op_8xy6(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    chip8->registers[0xF] = (chip8->registers[vx] & 0x1u);

    chip8->registers[vx] >>= 1;
}

void op_8xy7(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    if (chip8->registers[vy] > chip8->registers[vx])
        chip8->registers[0xF] = 1;
    else
        chip8->registers[0xF] = 0;

    chip8->registers[vx] = chip8->registers[vy] - chip8->registers[vx];
}

void op_8xyE(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    chip8->registers[0xF] = (chip8->registers[vx] & 0x80u) >> 7u;

    chip8->registers[vx] <<= 1;
}

void op_9xy0(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    if (chip8->registers[vx] != chip8->registers[vy])
        chip8->pc += 2;
}

void op_Annn(chip8_t* chip8)
{
    uint16_t nnn = make_nnn(chip8->opcode);
    chip8->index = nnn;
}

void op_Bnnn(chip8_t* chip8)
{
    uint16_t nnn = make_nnn(chip8->opcode);
    chip8->pc = chip8->registers[0] + nnn;
}

void op_Cxkk(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t kk = make_kk(chip8->opcode);

    std::default_random_engine e1(static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<uint8_t> uniform_dist{ 0, std::numeric_limits<uint8_t>::max() };

    chip8->registers[vx] = (uniform_dist(e1)) & kk;
}

void op_Dxyn(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);
    uint8_t height = chip8->opcode & 0x000Fu;

    // Wrap if going beyond screen boundaries
    uint8_t xPos = chip8->registers[vx] % VIDEO_WIDTH;
    uint8_t yPos = chip8->registers[vy] % VIDEO_HEIGHT;

    chip8->registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        uint8_t spriteByte = chip8->memory[chip8->index + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t* screenPixel = &chip8->video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel)
            {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF)
                {
                    chip8->registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void op_Ex9E(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t key = chip8->registers[vx];

    if (chip8->keypad[key])
        chip8->pc += 2;
}

void op_ExA1(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t key = chip8->registers[vx];

    if (!chip8->keypad[key])
        chip8->pc += 2;
}

void op_Fx07(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    chip8->registers[vx] = chip8->delay_timer;
}

void op_Fx0A(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    if (chip8->keypad[0])
    {
        chip8->registers[vx] = 0;
    }
    else if (chip8->keypad[1])
    {
        chip8->registers[vx] = 1;
    }
    else if (chip8->keypad[2])
    {
        chip8->registers[vx] = 2;
    }
    else if (chip8->keypad[3])
    {
        chip8->registers[vx] = 3;
    }
    else if (chip8->keypad[4])
    {
        chip8->registers[vx] = 4;
    }
    else if (chip8->keypad[5])
    {
        chip8->registers[vx] = 5;
    }
    else if (chip8->keypad[6])
    {
        chip8->registers[vx] = 6;
    }
    else if (chip8->keypad[7])
    {
        chip8->registers[vx] = 7;
    }
    else if (chip8->keypad[8])
    {
        chip8->registers[vx] = 8;
    }
    else if (chip8->keypad[9])
    {
        chip8->registers[vx] = 9;
    }
    else if (chip8->keypad[10])
    {
        chip8->registers[vx] = 10;
    }
    else if (chip8->keypad[11])
    {
        chip8->registers[vx] = 11;
    }
    else if (chip8->keypad[12])
    {
        chip8->registers[vx] = 12;
    }
    else if (chip8->keypad[13])
    {
        chip8->registers[vx] = 13;
    }
    else if (chip8->keypad[14])
    {
        chip8->registers[vx] = 14;
    }
    else if (chip8->keypad[15])
    {
        chip8->registers[vx] = 15;
    }
    else
    {
        chip8->pc -= 2;
    }
}

void op_Fx15(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    chip8->delay_timer = chip8->registers[vx];
}

void op_Fx18(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    chip8->sound_timer = chip8->registers[vx];
}

void op_Fx1E(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    chip8->index += chip8->registers[vx];
}

void op_Fx29(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t digit = chip8->registers[vx];

    chip8->index = static_cast<uint16_t>(FONTSET_START_ADDRESS + (5 * digit));
}

void op_Fx33(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t value = chip8->registers[vx];

    // Ones-place
    chip8->memory[chip8->index + 2] = value % 10;
    value /= 10;

    // Tens-place
    chip8->memory[chip8->index + 1] = value % 10;
    value /= 10;

    // Hundreds-place
    chip8->memory[chip8->index] = value % 10;
}

void op_Fx55(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    for (uint8_t i = 0; i <= vx; ++i)
    {
        chip8->memory[chip8->index + i] = chip8->registers[i];
    }
}

void op_Fx65(chip8_t* chip8)
{
    uint8_t vx = make_vx(chip8->opcode);

    for (uint8_t i = 0; i <= vx; ++i)
    {
        chip8->registers[i] = chip8->memory[chip8->index + i];
    }
}

func_ptr dispatch_table[0xF + 1];
func_ptr table0[0xE + 1];
func_ptr table8[0xE + 1];
func_ptr tableE[0xE + 1];
func_ptr tableF[0x65 + 1];

void opcodes_0(chip8_t* chip8)
{
    auto instruction =table0[chip8->opcode & 0x000Fu];
    instruction(chip8);
}

void opcodes_8(chip8_t* chip8)
{
    auto instruction =table8[chip8->opcode & 0x000Fu];
    instruction(chip8);
}

void opcodes_E(chip8_t* chip8)
{
    auto instruction =tableE[chip8->opcode & 0x000Fu];
    instruction(chip8);
}

void opcodes_F(chip8_t* chip8)
{
    auto instruction = tableF[chip8->opcode & 0x00FFu];
    instruction(chip8);
}

void op_null(chip8_t* chip8)
{}


void init_dispatch_table()
{
    dispatch_table[0x0] = opcodes_0;
    dispatch_table[0x1] = op_1nnn;
    dispatch_table[0x2] = op_2nnn;
    dispatch_table[0x3] = op_3xkk;
    dispatch_table[0x4] = op_4xkk;
    dispatch_table[0x5] = op_5xy0;
    dispatch_table[0x6] = op_6xkk;
    dispatch_table[0x7] = op_7xkk;
    dispatch_table[0x8] = opcodes_8;
    dispatch_table[0x9] = op_9xy0;
    dispatch_table[0xA] = op_Annn;
    dispatch_table[0xB] = op_Bnnn;
    dispatch_table[0xC] = op_Cxkk;
    dispatch_table[0xD] = op_Dxyn;
    dispatch_table[0xE] = opcodes_E;
    dispatch_table[0xF] = opcodes_F;

    for (size_t i = 0; i <= 0xE; i++) {
        table0[i] = op_null;
        table8[i] = op_null;
        tableE[i] = op_null;
    }

    table0[0x0] = op_00E0;
    table0[0xE] = op_00EE;

    table8[0x0] = op_8xy0;
    table8[0x1] = op_8xy1;
    table8[0x2] = op_8xy2;
    table8[0x3] = op_8xy3;
    table8[0x4] = op_8xy4;
    table8[0x5] = op_8xy5;
    table8[0x6] = op_8xy6;
    table8[0x7] = op_8xy7;
    table8[0xE] = op_8xyE;

    tableE[0x1] = op_ExA1;
    tableE[0xE] = op_Ex9E;

    for (size_t i = 0; i <= 0x65; i++) {
        tableF[i] = op_null;
    }

    tableF[0x07] = op_Fx07;
    tableF[0x0A] = op_Fx0A;
    tableF[0x15] = op_Fx15;
    tableF[0x18] = op_Fx18;
    tableF[0x1E] = op_Fx1E;
    tableF[0x29] = op_Fx29;
    tableF[0x33] = op_Fx33;
    tableF[0x55] = op_Fx55;
    tableF[0x65] = op_Fx65;
}

bytes_t read_program(const path_t& filepath)
{
    std::ifstream file(filepath, std::ios::out | std::ios::binary);
    if (!file.is_open())
        return {};

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

void load_rom(chip8_t* chip8, char const* filename)
{
    auto program = read_program(filename);
    if (program.size() > MAX_ROM_SIZE) {
        throw std::runtime_error("File size is bigger than max rom size.");
    } else if (program.size() <= 0) {
        throw std::runtime_error("No file or empty file.");
    }

    load_program(chip8->memory, program);
}

uint16_t fetch(chip8_t* chip8)
{
    return static_cast<uint16_t>((chip8->memory[chip8->pc] << 8u) | chip8->memory[chip8->pc + 1]);
}

void decode_and_execute(chip8_t* chip8)
{
    auto instruction = dispatch_table[(chip8->opcode & 0xF000u) >> 12u];
    instruction(chip8);
}

void cycle(chip8_t* chip8)
{
    // Fetch
    chip8->opcode = fetch(chip8);

    // Increment the PC before we execute anything
    chip8->pc += 2;

    // Decode and Execute
    decode_and_execute(chip8);

    // Decrement the delay timer if it's been set
    if (chip8->delay_timer > 0)
        chip8->delay_timer -= 1;

    // Decrement the sound timer if it's been set
    if (chip8->sound_timer > 0)
        chip8->sound_timer -= 1;
}

int main()
{
    auto chip8 = make_chip8();
    return 0;
}