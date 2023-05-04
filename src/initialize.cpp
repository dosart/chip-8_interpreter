#include "chip8.h"


static const unsigned int FONTSET_START_ADDRESS = 0x50;
static const unsigned int START_ADDRESS = 0x200;

static void init(chip8_t *chip8);

static void load_fonts(bytes_t &mem);

static bytes_t read_program(const std::filesystem::path &filepath);

static void load_program(bytes_t &mem, const bytes_t &program);

void initialize(chip8_t *chip8, const std::filesystem::path &filepath) {
  init(chip8);
  load_fonts(chip8->memory);

  auto program = read_program(filepath);
  load_program(chip8->memory, program);
}

static void init(chip8_t *chip8) {
  chip8->memory.reserve(4096);
  chip8->registers.reserve(16);
  chip8->stack.reserve(16);
  chip8->keypad.reserve(16);
  chip8->video.reserve(64 * 32);

  chip8->pc = START_ADDRESS;
}

static bytes_t read_program(const std::filesystem::path &filepath) {
  std::ifstream file(filepath.native(), std::ios::binary | std::ios::ate | std::ios::in);
  if (!file.is_open())
    throw std::runtime_error("cannot open input file");

  std::vector<uint8_t> bytes(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

  file.close();
  return bytes;
}

static void load_fonts(bytes_t &mem) {
  std::vector<uint8_t> fonts{
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
  std::memcpy(mem.data() + FONTSET_START_ADDRESS, fonts.data(), fonts.size());
}

static void load_program(bytes_t &mem, const bytes_t &program) {
  std::memcpy(mem.data() + START_ADDRESS, program.data(), program.size());
}