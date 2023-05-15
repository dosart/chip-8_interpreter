#include "chip8.h"
#include "platform.h"
#include <iostream>

extern void run_cycle(chip8_t *chip8);

int main() {
//    if (argc != 4)
//    {
//        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
//        std::exit(EXIT_FAILURE);
//    }

    int videoScale = 10; //std::stoi(argv[1]);
    int cycleDelay = 1; //std::stoi(argv[2]);
    char const* filename = "/home/dosart3/code/c++/chip-8_interpreter/cmake-build-debug/test_opcode.ch8";

    platform_t platform("CHIP-8 Emulator", 64 * videoScale, 32 * videoScale, 64, 32);

    chip8_t chip8;
    initialize(&chip8, filename);

    int videoPitch = sizeof(chip8.video.data()[0]) * 64;

    auto timePoint = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit)
    {
        quit = platform.process_input(chip8.keypad.data());

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - timePoint).count();

        if (dt > static_cast<float>(cycleDelay))
        {
            timePoint = currentTime;

            run_cycle(&chip8);

            platform.update(chip8.video.data(), videoPitch);
        }
    }

    return 0;
}