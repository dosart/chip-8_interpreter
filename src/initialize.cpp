//#include "chip8.h"
//
//extern void init_instructions();
//
//static void init(chip8_t *chip8);
//
//static void load_fonts(bytes_t &mem);
//
//static void read_program(chip8_t *chip8, const std::filesystem::path &filepath);
//
////static void load_program(bytes_t &mem, const bytes_t &program);
//
//void initialize(chip8_t *chip8, const std::filesystem::path &filepath) {
//  init(chip8);
//  init_instructions();
//  load_fonts(chip8->memory);
//
//  read_program(chip8, filepath);
//  //load_program(chip8->memory, program);
//}
//
//static void init(chip8_t *chip8) {
//  chip8->memory.reserve(4096);
//  chip8->registers.reserve(16);
//  chip8->stack.reserve(16);
//  chip8->keypad.reserve(16);
//  chip8->video.reserve(64 * 32);
//
//  chip8->pc = constants::START_ADDRESS;
//}
//
//static void read_program(chip8_t *chip8, const std::filesystem::path &filepath) {
//    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
//
//    if (file.is_open())
//    {
//        // Get size of file and allocate a buffer to hold the contents
//        std::streampos size = file.tellg();
//        char* buffer = new char[static_cast<unsigned long>(size)];
//
//        // Go back to the beginning of the file and fill the buffer
//        file.seekg(0, std::ios::beg);
//        file.read(buffer, size);
//        file.close();
//
//        // Load the ROM contents into the Chip8's memory, starting at 0x200
//        for (long i = 0; i < size; ++i)
//        {
//            chip8->memory.data()[constants::START_ADDRESS + i] = static_cast<unsigned char>(buffer[i]);
//        }
//
//        // Free the buffer
//        delete[] buffer;
//    }
//}
//
//static void load_fonts(bytes_t &mem) {
//  std::vector<uint8_t> fonts{
//      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
//      0x20, 0x60, 0x20, 0x20, 0x70, // 1
//      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
//      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
//      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
//      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
//      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
//      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
//      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
//      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
//      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
//      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
//      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
//      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
//      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
//      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
//  };
//  std::memcpy(mem.data() + constants::FONTSET_START_ADDRESS, fonts.data(), fonts.size());
//}
//
////static void load_program(bytes_t &mem, const bytes_t &program) {
////  std::memcpy(mem.data() + constants::START_ADDRESS, program.data(), program.size());
////}