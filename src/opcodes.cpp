#include <random>
#include "chip8.h"

/*------------------------------------------------------------------------------
//	Defining interface functions
//----------------------------------------------------------------------------*/

void run_cycle(chip8_t *chip8);

void init_instructions();

/*------------------------------------------------------------------------------
//	Implementation of static functions
//----------------------------------------------------------------------------*/

static inline uint8_t make_vx(uint16_t opcode) {
  return ((opcode & 0x0F00u) >> 8u);
}

static inline uint8_t make_vy(uint16_t opcode) {
  return ((opcode & 0x00F0u) >> 4u);
}

static inline uint8_t make_kk(uint16_t opcode) { return (opcode & 0x00FFu); }

static inline uint16_t make_nnn(uint16_t opcode) { return (opcode & 0x0FFFu); }

/**
 * @ingroup table
 *
 * @brief Clear the display.
 */
static void op_00E0(chip8_t *chip8) {
  std::memset(chip8->video.data(), 0, chip8->video.size());
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

  if (chip8->registers[vx] == kk) {
    chip8->pc += 2;
  }
}

/**
 * @ingroup table
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
 * @ingroup table
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

/**
 * @ingroup table
 *
 * @brief Set vx = vy.
 */
void op_8xy0(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] = chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx OR vy.
 */
static void op_8xy1(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] |= chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx AND vy.
 */
static void op_8xy2(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);

  chip8->registers[vx] &= chip8->registers[vy];
}

/**
 * @ingroup table
 *
 * @brief Set vx = vx XOR vy.
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
  uint8_t vf = 0xFu;

  uint16_t sum = chip8->registers[vx] + chip8->registers[vy];
  if (sum > 255)
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
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
  uint8_t vf = 0xFu;

  if (chip8->registers[vx] > chip8->registers[vy])
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
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
  uint8_t vf = 0xFu;

  chip8->registers[vf] = (chip8->registers[vx] & 0x1u);
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
  uint8_t vf = 0xFu;

  if (chip8->registers[vy] > chip8->registers[vx])
    chip8->registers[vf] = 1;
  else
    chip8->registers[vf] = 0;
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
  uint8_t vf = 0xFu;

  chip8->registers[vf] = (chip8->registers[vx] & 0x80) >> 7u;
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

  if (chip8->registers[vy] != chip8->registers[vx])
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
  uint8_t v0 = 0;
  chip8->pc = chip8->registers[v0] + nnn;
}

/**
 * @ingroup table
 *
 * @brief Set vx = random byte AND kk.
 */
static void op_Cxkk(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t kk = make_kk(chip8->opcode);

  std::random_device rd;
  std::uniform_int_distribution<uint8_t> dist(0, 246);
  uint8_t random_byte = (dist(rd));

  chip8->registers[vx] = random_byte & kk;
}

using index_t = std::vector<int>::size_type;

/**
 * @ingroup table
 *
 * @brief Display n-byte sprite starting at memory location I at (Vx, Vy), set
 * VF = collision.
 */
static void op_Dxyn(chip8_t *chip8) {
  uint8_t vx = make_vx(chip8->opcode);
  uint8_t vy = make_vy(chip8->opcode);
  uint8_t vf = 0xFu;

  uint8_t height = chip8->opcode & 0x000Fu;

  uint8_t x_coord = chip8->registers[vx] % constants::VIDEO_WIDTH;
  uint8_t y_coord = chip8->registers[vy] % constants::VIDEO_HEIGHT;
  chip8->registers[vf] = 0;

  for (uint8_t row = 0; row < height; ++row) {
    uint8_t spriteByte = chip8->memory[chip8->index + row];

    for (uint8_t col = 0; col < 8; ++col) {
      uint8_t spritePixel = spriteByte & (0x80u >> col);

      auto index = static_cast<index_t>(
          (y_coord + row) * constants::VIDEO_WIDTH + (x_coord + col));
      uint32_t *screenPixel = &chip8->video[index];

      // Sprite pixel is on
      if (spritePixel) {
        // Screen pixel also on - collision
        if (*screenPixel == 0xFFFFFFFFu)
          chip8->registers[vf] = 1;

        // Effectively XOR with the sprite pixel
        *screenPixel ^= 0xFFFFFFFFu;
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

  chip8->index =
      static_cast<uint16_t>(constants::FONTSET_START_ADDRESS + (5 * digit));
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
  for (uint8_t i = 0; i <= vx; ++i)
    chip8->registers[i] = chip8->memory[chip8->index + i];
}

static void op_null([[maybe_unused]] chip8_t *chip8) {}

using func_ptr = void (*)(chip8_t *chip8);

static std::array<func_ptr, 0xF> table0 = {};
static void opcodes0(chip8_t *chip8) { table0[chip8->opcode & 0x000Fu]; }

static std::array<func_ptr, 0xF> table8 = {};
static void opcodes8(chip8_t *chip8) { table8[chip8->opcode & 0x000Fu]; }

static std::array<func_ptr, 0xF> tableE = {};
static void opcodesE(chip8_t *chip8) { tableE[chip8->opcode & 0x000Fu]; }

static std::array<func_ptr, 0x66> tableF = {};
static void opcodesF(chip8_t *chip8) { tableF[chip8->opcode & 0x00FFu]; }

static std::array<func_ptr, 0x10> table = {};

/**
 * @ingroup table
 *
 * @brief Return current instruction.
 * The address of the current instruction is stored in the pc register.
 */
static uint16_t fetch(chip8_t *chip8) {
        return (static_cast<uint16_t>((chip8->memory[chip8->pc] << 8u) | (chip8->memory[chip8->pc + 1])));
}

/**
 * @ingroup table
 *
 * @brief Decode and execute current instruction.
 * The current instruction is stored in the chip8->opcode.
 */
static void decode_and_execute(chip8_t *chip8) {
  auto index = (chip8->opcode & 0xF000u) >> 12u;
  auto instruction = table[index];
  instruction(chip8);
}

/*------------------------------------------------------------------------------
//	Implementation of interface functions
//----------------------------------------------------------------------------*/

/**
 * @ingroup table
 *
 * @brief The main loop(fetch, decode and execute instruction).
 */
void run_cycle(chip8_t *chip8) {
  chip8->opcode = fetch(chip8);
  decode_and_execute(chip8);

  if (chip8->delay_timer > 0)
    chip8->delay_timer -= 1;

  if (chip8->sound_timer > 0)
    chip8->sound_timer -= 1;
}

/**
 * @ingroup table
 *
 * @brief Initialize the instruction table.
 */
void init_instructions() {
  table[0x0] = opcodes0;
  table[0x1] = op_1nnn;
  table[0x2] = op_2nnn;
  table[0x3] = op_3xkk;
  table[0x4] = op_4xkk;
  table[0x5] = op_5xy0;
  table[0x6] = op_6xkk;
  table[0x7] = op_7xkk;
  table[0x8] = opcodes8;
  table[0x9] = op_9xy0;
  table[0xA] = op_Annn;
  table[0xB] = op_Bnnn;
  table[0xC] = op_Cxkk;
  table[0xD] = op_Dxyn;
  table[0xE] = opcodesE;
  table[0xF] = opcodesF;

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
