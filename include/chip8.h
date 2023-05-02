
#ifndef CHIP_8_INTERPRETER_CHIP8_H
#define CHIP_8_INTERPRETER_CHIP8_H

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

using bytes_t = std::vector<uint8_t>;

bytes_t load_rom(std::filesystem::path filepath);

#endif // CHIP_8_INTERPRETER_CHIP8_H
