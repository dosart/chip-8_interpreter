#include "chip8.h"

static void init_dispatch_table();

static unsigned char has_initialized = 0;

static func_ptr dispatch_table[0xF + 1];
static func_ptr table0[0xE + 1];
static func_ptr table8[0xE + 1];
static func_ptr tableE[0xE + 1];
static func_ptr tableF[0x65 + 1];

static void init_dispatcher() {
  init_dispatch_table();
  has_initialized = 1;
}

func_ptr *make_dispatcher() {
  if (!has_initialized) {
    init_dispatcher();
  }
  return dispatch_table;
}

static uint8_t make_vx(uint16_t opcode) { return ((opcode & 0x0F00u) >> 8u); }

static uint8_t make_vy(uint16_t opcode) { return ((opcode & 0x00F0u) >> 4u); }

static uint8_t make_kk(uint16_t opcode) { return (opcode & 0x00FFu); }

static uint16_t make_nnn(uint16_t opcode) { return (opcode & 0x0FFFu); }

/**
 * @ingroup table
 *
 * @brief Clear the display.
 */
static void op_00E0(chip8_t *chip8) {
  std::memset(chip8->video, 0, sizeof(chip8->video));
}

/**
 * @ingroup table
 *
 * @brief Return from a subroutine.
 */
static void op_00EE(chip8_t *chip8) {
  chip8->sp -= 1;
  chip8->pc = chip8->stack[chip8->sp];
}

/**
 * @ingroup table
 *
 * @brief Jump to location nnn.
 */
static void op_1nnn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);
  chip8->pc = nnn;
}

/**
 * @ingroup table
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
 * @ingroup table
 *
 * @brief Skip next instruction if vx = kk.
 */
static void op_3xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  if (chip8->registers[vx] == kk)
    chip8->pc += 2;
}

/**
 * @ingroup table
 *
 * @brief Skip next instruction if vx != kk.
 */
static void op_4xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  if (chip8->registers[vx] != kk)
    chip8->pc += 2;
}

/**
 * @ingroup table
 *
 * @brief Skip next instruction if vx = vy.
 */
static void op_5xy0(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  if (chip8->registers[vx] == chip8->registers[vy])
    chip8->pc += 2;
}

/**
 * @ingroup table
 *
 * @brief Set vx = kk.
 */
static void op_6xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  chip8->registers[vx] = kk;
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx + kk.
 */
static void op_7xkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  chip8->registers[vx] += kk;
}

static void op_8xy0(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] = chip8->registers[vy];
}

static void op_8xy1(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] |= chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vy.
 */
static void op_8xy2(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] &= chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx OR vy.
 */
static void op_8xy3(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] ^= chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx + vy, set vf = carry.
 */
static void op_8xy4(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  uint16_t sum = chip8->registers[vx] + chip8->registers[vy];

  if (sum > 255U)
    chip8->registers[0xF] = 1;
  else
    chip8->registers[0xF] = 0;

  chip8->registers[vx] = sum & 0xFFu;
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx - vy, set vf = NOT borrow.
 * If vx > vy, then vf is set to 1, otherwise 0. Then vy is subtracted from vx,
 * and the results stored in vx.
 */
static void op_8xy5(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  if (chip8->registers[vx] > chip8->registers[vy])
    chip8->registers[0xF] = 1;
  else
    chip8->registers[0xF] = 0;

  chip8->registers[vx] -= chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx - vy, set vf = NOT borrow.
 * If vx > vy, then vf is set to 1, otherwise 0. Then vy is subtracted from vx,
 * and the results stored in vx.
 */
static void op_8xy6(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  chip8->registers[0xF] = (chip8->registers[vx] & 0x1u);
  chip8->registers[vx] >>= 1;
}

/**
 * @ingroup table
 *
 * @brief Set vx = vy - vx, set vf = NOT borrow.
 * If vy > vx, then vf is set to 1, otherwise 0. Then vx is subtracted from vy,
 * and the results stored in vx.
 */
static void op_8xy7(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  if (chip8->registers[vy] > chip8->registers[vx])
    chip8->registers[0xF] = 1;
  else
    chip8->registers[0xF] = 0;

  chip8->registers[vx] = chip8->registers[vy] - chip8->registers[vx];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx SHL 1.
 * If the most-significant bit of vx is 1, then vf is set to 1, otherwise to 0.
 * Then vx is multiplied by 2.
 */
static void op_8xyE(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  chip8->registers[0xF] = (chip8->registers[vx] & 0x80u) >> 7u;

  chip8->registers[vx] <<= 1;
}

/**
 * @ingroup table
 *
 * @brief Skip next instruction if vx != vy.
 */
static void op_9xy0(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  if (chip8->registers[vx] != chip8->registers[vy])
    chip8->pc += 2;
}

/**
 * @ingroup table
 *
 * @brief Set I = nnn.
 */
static void op_Annn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);
  chip8->index = nnn;
}

/**
 * @ingroup table
 *
 * @brief Jump to location nnn + v0.
 */
static void op_Bnnn(chip8_t *chip8) {
  uint16_t nnn = make_nnn(chip8->opcode);
  chip8->pc = chip8->registers[0] + nnn;
}

/**
 * @ingroup table
 *
 * @brief Set vx = random byte AND kk.
 */
static void op_Cxkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  std::default_random_engine e1(static_cast<unsigned long>(
      std::chrono::system_clock::now().time_since_epoch().count()));
  std::uniform_int_distribution<uint8_t> uniform_dist{
      0, std::numeric_limits<uint8_t>::max()};

  chip8->registers[vx] = (uniform_dist(e1)) & kk;
}

struct sprite_t {
  uint8_t sprite_x;
  uint8_t sprite_y;
  uint8_t sprite_height;

  sprite_t(chip8_t *chip8) {
    uint8_t vx = make_vx(chip8->opcode);
    uint8_t vy = make_vy(chip8->opcode);

    sprite_x = chip8->registers[vx] % VIDEO_WIDTH;
    sprite_y = chip8->registers[vy] % VIDEO_HEIGHT;
    sprite_height = chip8->opcode & 0x000Fu;
  }
};

static uint32_t *get_video_pixel(chip8_t *chip8, sprite_t *sprite,
                                 unsigned int row, unsigned int col) {
  return &chip8->video[(sprite->sprite_y + row) * VIDEO_WIDTH +
                       (sprite->sprite_x + col)];
}

static uint8_t get_sprite_pixel(uint8_t sprite_row, uint8_t col) {
  return sprite_row & (0x80u >> col);
}

/**
 * @ingroup table
 *
 * @brief Display n-byte sprite starting at memory location I at (Vx, Vy), set
 * VF = collision.
 */
static void op_Dxyn(chip8_t *chip8) {
  // reset collision bit
  chip8->registers[0xF] = 0;

  /*
   * Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a
   * height of N+1 pixels. Each row of 8 pixels is read as bit-coded starting
   * from memory location I VF is set to 1 if any screen pixels are flipped from
   * set to unset when the sprite is drawn, and to 0 if that doesn’t happen
   * */
  sprite_t sprite(chip8);
  for (uint8_t row = 0; row < sprite.sprite_height; ++row) {
    uint8_t sprite_row = chip8->memory[chip8->index + row];
    for (uint8_t col = 0; col < 8; ++col) {
      uint8_t sprite_pixel = get_sprite_pixel(sprite_row, col);
      uint32_t *video_pixel = get_video_pixel(chip8, &sprite, row, col);
      // Sprite pixel is on
      if (sprite_pixel) {
        // Video pixel also on - collision
        if (*video_pixel == 0xFFFFFFFF) {
          chip8->registers[0xF] = 1;
        }
        // Effectively XOR with the sprite pixel
        *video_pixel ^= 0xFFFFFFFF;
      }
    }
  }
}

/**
 * @ingroup table
 *
 * @brief Skip next instruction if key with the value of Vx is pressed.
 */
static void op_Ex9E(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t key = chip8->registers[vx];

  if (chip8->keypad[key])
    chip8->pc += 2;
}

/**
 * @ingroup table
 *
 * @brief Skip next instruction if key with the value of Vx is not pressed.
 */
static void op_ExA1(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t key = chip8->registers[vx];

  if (!chip8->keypad[key])
    chip8->pc += 2;
}

/**
 * @ingroup table
 *
 * @brief Skip next instruction if key with the value of Vx is not pressed.
 */
static void op_Fx07(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  chip8->registers[vx] = chip8->delay_timer;
}

/**
 * @ingroup table
 *
 * @brief Wait for a key press, store the value of the key in Vx.
 */
static void op_Fx0A(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  if (chip8->keypad[0]) {
    chip8->registers[vx] = 0;
  } else if (chip8->keypad[1]) {
    chip8->registers[vx] = 1;
  } else if (chip8->keypad[2]) {
    chip8->registers[vx] = 2;
  } else if (chip8->keypad[3]) {
    chip8->registers[vx] = 3;
  } else if (chip8->keypad[4]) {
    chip8->registers[vx] = 4;
  } else if (chip8->keypad[5]) {
    chip8->registers[vx] = 5;
  } else if (chip8->keypad[6]) {
    chip8->registers[vx] = 6;
  } else if (chip8->keypad[7]) {
    chip8->registers[vx] = 7;
  } else if (chip8->keypad[8]) {
    chip8->registers[vx] = 8;
  } else if (chip8->keypad[9]) {
    chip8->registers[vx] = 9;
  } else if (chip8->keypad[10]) {
    chip8->registers[vx] = 10;
  } else if (chip8->keypad[11]) {
    chip8->registers[vx] = 11;
  } else if (chip8->keypad[12]) {
    chip8->registers[vx] = 12;
  } else if (chip8->keypad[13]) {
    chip8->registers[vx] = 13;
  } else if (chip8->keypad[14]) {
    chip8->registers[vx] = 14;
  } else if (chip8->keypad[15]) {
    chip8->registers[vx] = 15;
  } else {
    chip8->pc -= 2;
  }
}

/**
 * @ingroup table
 *
 * @brief Set delay timer = vx.
 */
static void op_Fx15(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  chip8->delay_timer = chip8->registers[vx];
}

/**
 * @ingroup table
 *
 * @brief Set sound timer = vx.
 */
static void op_Fx18(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  chip8->sound_timer = chip8->registers[vx];
}

/**
 * @ingroup table
 *
 * @brief Set I = I + Vx.
 */
static void op_Fx1E(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  chip8->index += chip8->registers[vx];
}

/**
 * @ingroup table
 *
 * @brief Set I = location of sprite for digit Vx.
 */
static void op_Fx29(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t digit = chip8->registers[vx];

  chip8->index = static_cast<uint16_t>(FONTSET_START_ADDRESS + (5 * digit));
}

/**
 * @ingroup table
 *
 * @brief Store BCD representation of Vx in memory locations I, I+1, and I+2.
 * The interpreter takes the decimal value of Vx, and places the hundreds digit
 * in memory at location in I, the tens digit at location I+1, and the ones
 * digit at location I+2.
 */
static void op_Fx33(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t value = chip8->registers[vx];

  chip8->memory[chip8->index + 2] = value % 10;
  value /= 10;

  chip8->memory[chip8->index + 1] = value % 10;
  value /= 10;

  chip8->memory[chip8->index] = value % 10;
}

static void op_Fx55(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  for (uint8_t i = 0; i <= vx; ++i)
    chip8->memory[chip8->index + i] = chip8->registers[i];
}

static void op_Fx65(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  for (uint8_t i = 0; i <= vx; ++i) {
    chip8->registers[i] = chip8->memory[chip8->index + i];
  }
}

static void opcodes_0(chip8_t *chip8) {
  auto instruction = table0[chip8->opcode & 0x000Fu];
  instruction(chip8);
}

static void opcodes_8(chip8_t *chip8) {
  auto instruction = table8[chip8->opcode & 0x000Fu];
  instruction(chip8);
}

static void opcodes_E(chip8_t *chip8) {
  auto instruction = tableE[chip8->opcode & 0x000Fu];
  instruction(chip8);
}

static void opcodes_F(chip8_t *chip8) {
  auto instruction = tableF[chip8->opcode & 0x00FFu];
  instruction(chip8);
}

static void op_null([[maybe_unused]] chip8_t *chip8) {}

static void init_dispatch_table() {
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