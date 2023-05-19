#pragma once

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <random>
#include <vector>

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

chip8_ptr_t make_chip8();
void load_rom(chip8_t *chip8, char const *filename);
void run_cycle(chip8_t *chip8);