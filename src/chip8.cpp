
#include "chip8.h"
#include <fstream>

static void init(chip8_t *chip8);
static void load_fonset(uint8_t *memory);

chip8_ptr_t make_chip8() {
  auto chip8 = std::make_unique<chip8_t>();
  init(chip8.get());
  return chip8;
}

static void init(chip8_t *chip8) {
  if (chip8 == nullptr)
    return;

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
}

static void load_fonset(uint8_t *memory) {
  uint8_t chip8_fontset[FONTSET_SIZE] = {
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

static bytes_t read_program(const path_t &filepath);
static void load_program(uint8_t *memory, const bytes_t &program);

void load_rom(chip8_t *chip8, char const *filename) {
  auto program = read_program(filename);
  if (program.size() > MAX_ROM_SIZE) {
    throw std::runtime_error("File size is bigger than max rom size.");
  } else if (program.size() <= 0) {
    throw std::runtime_error("No file or empty file.");
  }

  load_program(chip8->memory, program);
}

static uint16_t fetch(chip8_t *chip8);
static void decode_and_execute(chip8_t *chip8);

void run_cycle(chip8_t *chip8) {
  chip8->opcode = fetch(chip8);
  chip8->pc += 2;
  decode_and_execute(chip8);

  if (chip8->delay_timer > 0)
    chip8->delay_timer -= 1;
  if (chip8->sound_timer > 0)
    chip8->sound_timer -= 1;
}

static uint16_t fetch(chip8_t *chip8) {
  return static_cast<uint16_t>((chip8->memory[chip8->pc] << 8u) |
                               chip8->memory[chip8->pc + 1]);
}

extern func_ptr *make_dispatcher();

static void decode_and_execute(chip8_t *chip8) {
  auto dispatcher = make_dispatcher();
  auto instruction = dispatcher[(chip8->opcode & 0xF000u) >> 12u];
  instruction(chip8);
}

static bytes_t read_program(const path_t &filepath) {
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

static void load_program(uint8_t *memory, const bytes_t &program) {
    std::memcpy(memory + PROGRAM_START_ADDRESS, program.data(), program.size());
}
