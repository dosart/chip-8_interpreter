//#include "chip8.h"
//#include "platform.h"
//#include <iostream>
//
//extern void run_cycle(chip8_t *chip8);
//
//int main() {
////    if (argc != 4)
////    {
////        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
////        std::exit(EXIT_FAILURE);
////    }
//
//    int videoScale = 10; //std::stoi(argv[1]);
//    int cycleDelay = 1; //std::stoi(argv[2]);
//    char const* filename = "/home/dosart3/code/c++/chip-8_interpreter/cmake-build-debug/test_opcode.ch8";
//
//    platform_t platform("CHIP-8 Emulator", 64 * videoScale, 32 * videoScale, 64, 32);
//
//    chip8_t chip8;
//    initialize(&chip8, filename);
//
//    int videoPitch = sizeof(chip8.video.data()[0]) * 64;
//
//    auto timePoint = std::chrono::high_resolution_clock::now();
//    bool quit = false;
//
//    while (!quit)
//    {
//        quit = platform.process_input(chip8.keypad.data());
//
//        auto currentTime = std::chrono::high_resolution_clock::now();
//        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - timePoint).count();
//
//        if (dt > static_cast<float>(cycleDelay))
//        {
//            timePoint = currentTime;
//
//            run_cycle(&chip8);
//
//            platform.update(chip8.video.data(), videoPitch);
//        }
//    }
//
//    return 0;
//}

#include "chip8.h"
#include "platform.h"
#include <chrono>
#include <iostream>


int main()
{
//    if (argc != 4)
//    {
//        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
//        std::exit(EXIT_FAILURE);
//    }
//
//    int videoScale = std::stoi(argv[1]);
  //    int cycleDelay = std::stoi(argv[2]);
  //    char const* romFilename = argv[3];

  int videoScale = 10; // std::stoi(argv[1]);
  int cycleDelay = 1;  // std::stoi(argv[2]);
  char const *romFilename = "/home/dosart3/code/c++/chip-8_interpreter/"
                            "cmake-build-debug/test_opcode.ch8";

  platform_t platform(
      "CHIP-8 Emulator",
      static_cast<int>(VIDEO_WIDTH * static_cast<unsigned int>(videoScale)),
      static_cast<int>(VIDEO_HEIGHT * static_cast<unsigned int>(videoScale)),
      VIDEO_WIDTH, VIDEO_HEIGHT);

  auto chip8 = make_chip8();
  load_rom(chip8.get(), romFilename);

  int videoPitch = sizeof(chip8->video[0]) * VIDEO_WIDTH;

  auto lastCycleTime = std::chrono::high_resolution_clock::now();
  bool quit = false;

  while (!quit) {
    quit = platform.process_input(chip8->keypad);

    auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > static_cast<float>(cycleDelay))
        {
            lastCycleTime = currentTime;

            run_cycle(chip8.get());

            platform.update(chip8->video, videoPitch);
        }
    }

    return 0;
}