#include "chip8.h"

static inline uint8_t make_vx(uint16_t opcode) {
  return ((opcode & 0x0F00) >> 8);
}

static inline uint8_t make_vy(uint16_t opcode) {
  return ((opcode & 0x00F0) >> 4);
}

static inline uint8_t make_kk(uint16_t opcode) { return (opcode & 0x00FF); }

static inline uint16_t make_nnn(uint16_t opcode) { return (opcode & 0x0FFF); }

/**
 * @ingroup opcodes
 *
 * @brief Clear the display.
 */
static void op_00E0(chip8_t *chip8) {
  std::memset(chip8->video.data(), 0, chip8->video.size());
}

/**
 * @ingroup opcodes
 *
 * @brief Return from a subroutine.
 */
static void op_00EE(chip8_t *chip8) {
  chip8->sp -= 1;
  chip8->pc = chip8->stack[chip8->sp];
}

/**
 * @ingroup opcodes
 *
 * @brief Jump to location nnn.
 */
static void op_1nnn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);
  chip8->pc = nnn;
}

/**
 * @ingroup opcodes
 *
 * @brief Call subroutine at nnn.
 */
static void op_2nnn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);

  chip8->stack[chip8->sp] = chip8->pc;
  chip8->sp += 1;

  chip8->pc = nnn;
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if Vx = kk.
 */
static void op_3xkk(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  if (chip8->registers[Vx] == kk) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if Vx != kk.
 */
static void op_4xkk(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  if (chip8->registers[Vx] != kk) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if Vx = Vy.
 */
static void op_5xy0(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);

  if (chip8->registers[Vx] == chip8->registers[Vy]) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = kk.
 */
static void op_6xkk(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  chip8->registers[Vx] = kk;
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx + kk.
 */
static void op_7xkk(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  chip8->registers[Vx] += kk;
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx OR Vy.
 */
static void op_8xy1(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);

  chip8->registers[Vx] |= chip8->registers[Vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx AND Vy.
 */
static void op_8xy2(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);

  chip8->registers[Vx] &= chip8->registers[Vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx XOR Vy.
 */
static void op_8xy3(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);

  chip8->registers[Vx] ^= chip8->registers[Vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx + Vy, set VF = carry.
 */
static void op_8xy4(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);
  uint8_t Vf = 0xF;

  uint16_t sum = chip8->registers[Vx] + chip8->registers[Vy];
  if (sum > 255u)
    chip8->registers[Vf] = 1;
  else
    chip8->registers[Vf] = 0;
  chip8->registers[Vx] = sum & 0xFFu;
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx - Vy, set VF = NOT borrow.
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx,
 * and the results stored in Vx.
 */
static void op_8xy5(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);
  uint8_t Vf = 0xF;

  if (chip8->registers[Vx] > chip8->registers[Vy])
    chip8->registers[Vf] = 1;
  else
    chip8->registers[Vf] = 0;
  chip8->registers[Vx] -= chip8->registers[Vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set Vx = Vx - Vy, set VF = NOT borrow.
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx,
 * and the results stored in Vx.
 */
static void op_8xy6(chip8_t *chip8) {
  uint8_t Vx = make_vx(chip8->opcode);
  uint8_t Vy = make_vy(chip8->opcode);
  uint8_t Vf = 0xF;

  if (chip8->registers[Vx] > chip8->registers[Vy])
    chip8->registers[Vf] = 1;
  else
    chip8->registers[Vf] = 0;
  chip8->registers[Vx] -= chip8->registers[Vy];
}
