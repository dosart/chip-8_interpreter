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
 * @brief Skip next instruction if vx = kk.
 */
static void op_3xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  if (chip8->registers[vx] == kk) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if vx != kk.
 */
static void op_4xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  if (chip8->registers[vx] != kk) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if vx = vy.
 */
static void op_5xy0(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  if (chip8->registers[vx] == chip8->registers[vy]) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = kk.
 */
static void op_6xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  chip8->registers[vx] = kk;
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx + kk.
 */
static void op_7xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  chip8->registers[vx] += kk;
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx OR vy.
 */
static void op_8xy1(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] |= chip8->registers[vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx AND vy.
 */
static void op_8xy2(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] &= chip8->registers[vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx XOR vy.
 */
static void op_8xy3(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] ^= chip8->registers[vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx + vy, set vf = carry.
 */
static void op_8xy4(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);
  uint8_t vf = 0xF;

  uint16_t sum = chip8->registers[vx] + chip8->registers[vy];
  if (sum > 255u)
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
  chip8->registers[vx] = sum & 0xFFu;
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx - vy, set vf = NOT borrow.
 * If vx > vy, then vf is set to 1, otherwise 0. Then vy is subtracted from vx,
 * and the results stored in vx.
 */
static void op_8xy5(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);
  uint8_t vf = 0xF;

  if (chip8->registers[vx] > chip8->registers[vy])
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
  chip8->registers[vx] -= chip8->registers[vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx - vy, set vf = NOT borrow.
 * If vx > vy, then vf is set to 1, otherwise 0. Then vy is subtracted from vx,
 * and the results stored in vx.
 */
static void op_8xy6(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);
  uint8_t vf = 0xF;

  if (chip8->registers[vx] > chip8->registers[vy])
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
  chip8->registers[vx] -= chip8->registers[vy];
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vy - vx, set vf = NOT borrow.
 * If vy > vx, then vf is set to 1, otherwise 0. Then vx is subtracted from vy,
 * and the results stored in vx.
 */
static void op_8xy7(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);
  uint8_t vf = 0xF;

  if (chip8->registers[vy] > chip8->registers[vx])
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
  chip8->registers[vx] = chip8->registers[vy] - chip8->registers[vx];
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = vx SHL 1.
 * If the most-significant bit of vx is 1, then vf is set to 1, otherwise to 0.
 * Then vx is multiplied by 2.
 */
static void op_8xyE(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vf = 0xF;

  chip8->registers[vf] = (chip8->registers[vx] & 0x0F00) >> 7;
  chip8->registers[vx] <<= 1;
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if vx != vy.
 */
static void op_9xy0(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  if (chip8->registers[vy] != chip8->registers[vx])
    chip8->pc += 2;
}

/**
 * @ingroup opcodes
 *
 * @brief Set I = nnn.
 */
static void op_Annn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);
  chip8->index = nnn;
}

/**
 * @ingroup opcodes
 *
 * @brief Jump to location nnn + v0.
 */
static void op_Bnnn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);
  uint8_t v0 = 0;
  chip8->pc = chip8->registers[v0] + nnn;
}

/**
 * @ingroup opcodes
 *
 * @brief Set vx = random byte AND kk.
 */
static void op_Cxkk(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);
  uint8_t random_byte = (static_cast<uint8_t>(std::rand() % 256));

  chip8->registers[vx] = random_byte & kk;
}


/**
 * @ingroup opcodes
 *
 * @brief Display n-byte sprite starting at memory location I at (Vx, Vy), set
 * VF = collision.
 */
static void op_Dxyn(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);
  uint8_t vf = 0xF;

  uint8_t height = chip8->opcode & 0x000F;

  uint8_t x_coord = chip8->registers[vx] % constants::VIDEO_WIDTH;
  uint8_t y_coord = chip8->registers[vy] % constants::VIDEO_HEIGHT;
  chip8->registers[vf] = 0;

  for (uint8_t row = 0; row < height; ++row) {
    uint8_t spriteByte = chip8->memory[chip8->index + row];

    for (uint8_t col = 0; col < 8; ++col) {
      uint8_t spritePixel = spriteByte & (0x80u >> col);
      uint32_t *screenPixel =
          &chip8->video[(y_coord + row) * constants::VIDEO_WIDTH + (x_coord + col)];

      // Sprite pixel is on
      if (spritePixel) {
        // Screen pixel also on - collision
        if (*screenPixel == 0xFFFFFFFF)
          chip8->registers[vf] = 1;

        // Effectively XOR with the sprite pixel
        *screenPixel ^= 0xFFFFFFFF;
      }
    }
  }
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if key with the value of Vx is pressed.
 */
static void op_Ex9E(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  uint8_t key = chip8->registers[vx];

  if (chip8->keypad[key])
    chip8->pc += 2;
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if key with the value of Vx is not pressed.
 */
static void op_ExA1(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  uint8_t key = chip8->registers[vx];

  if (!chip8->keypad[key])
    chip8->pc += 2;
}

/**
 * @ingroup opcodes
 *
 * @brief Skip next instruction if key with the value of Vx is not pressed.
 */
static void op_Fx07(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  chip8->registers[vx] = chip8->delay_timer;
}

/**
 * @ingroup opcodes
 *
 * @brief Wait for a key press, store the value of the key in Vx.
 */
static void op_Fx0A(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);

  if (chip8->keypad[0])
    chip8->registers[vx] = 0;
  else if (chip8->keypad[1])
    chip8->registers[vx] = 1;
  else if (chip8->keypad[2])
    chip8->registers[vx] = 2;
  else if (chip8->keypad[3])
    chip8->registers[vx] = 3;
  else if (chip8->keypad[4])
    chip8->registers[vx] = 4;
  else if (chip8->keypad[5])
    chip8->registers[vx] = 5;
  else if (chip8->keypad[6])
    chip8->registers[vx] = 6;
  else if (chip8->keypad[7])
    chip8->registers[vx] = 7;
  else if (chip8->keypad[8])
    chip8->registers[vx] = 8;
  else if (chip8->keypad[9])
    chip8->registers[vx] = 9;
  else if (chip8->keypad[10])
    chip8->registers[vx] = 10;
  else if (chip8->keypad[11])
    chip8->registers[vx] = 11;
  else if (chip8->keypad[12])
    chip8->registers[vx] = 12;
  else if (chip8->keypad[13])
    chip8->registers[vx] = 13;
  else if (chip8->keypad[14])
    chip8->registers[vx] = 14;
  else if (chip8->keypad[15])
    chip8->registers[vx] = 15;
  else
    chip8->pc -= 2;
}

/**
 * @ingroup opcodes
 *
 * @brief Set sound timer = Vx.
 */
static void op_Fx15(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  chip8->sound_timer = chip8->registers[vx];
}

/**
 * @ingroup opcodes
 *
 * @brief Set I = I + Vx.
 */
static void op_Fx1E(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  chip8->index += chip8->registers[vx];
}

/**
 * @ingroup opcodes
 *
 * @brief Set I = location of sprite for digit Vx.
 */
static void op_Fx29(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  uint8_t digit = chip8->registers[vx];

  chip8->index = constants::FONTSET_START_ADDRESS + (5 * digit);
}

/**
 * @ingroup opcodes
 *
 * @brief Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * The interpreter takes the decimal value of Vx, and places the hundreds digit
 * in memory at location in I, the tens digit at location I+1, and the ones
 * digit at location I+2.
 */
static void op_Fx33(chip8_t *chip8) {
  uint16_t vx = make_vx(chip8->opcode);
  uint8_t value = chip8->registers[vx];

  chip8->memory[chip8->index + 2] = value % 10;
  value /= 10;

  chip8->memory[chip8->index + 1] = value % 10;
  value /= 10;

  chip8->memory[chip8->index] = value % 10;
}