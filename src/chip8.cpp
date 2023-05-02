#include "chip8.h"

bytes_t load_rom(std::filesystem::path filepath)
{
  std::ifstream file(filepath.native(), std::ios::binary | std::ios::ate | std::ios::in);
  if(!file.is_open())
    throw std::runtime_error("cannot open input file");

  std::vector<uint8_t> bytes(
      (std::istreambuf_iterator<char>(file)),
      (std::istreambuf_iterator<char>()));

  file.close();
  return bytes;
}
