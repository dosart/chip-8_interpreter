
#ifndef CHIP_8_INTERPRETER_CHIP8_H
#define CHIP_8_INTERPRETER_CHIP8_H

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace constants
{
constexpr unsigned int FONTSET_START_ADDRESS = {0x50};
constexpr unsigned int  START_ADDRESS { 0x200 };
constexpr uint8_t VIDEO_WIDTH { 64 };
constexpr uint8_t VIDEO_HEIGHT { 32 };
}

using bytes_t = std::vector<uint8_t>;
using words_t = std::vector<uint16_t>;
using double_words_t = std::vector<uint32_t>;

struct chip8_t {
  bytes_t memory{};
  bytes_t registers{};
  bytes_t keypad{};
  words_t stack{};

  uint16_t index{};
  uint16_t pc{};
  uint8_t sp{};

  uint8_t delay_timer{};
  uint8_t sound_timer{};

  double_words_t video{};
  uint16_t opcode;
};

void initialize(chip8_t *chip8, const std::filesystem::path &filepath);

#endif // CHIP_8_INTERPRETER_CHIP8_H