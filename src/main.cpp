#include "chip8.h"
#include "platform.h"
#include <chrono>

int main() {
  //    if (argc != 4)
  //    {
  //        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
  //        std::exit(EXIT_FAILURE);
  //    }
  //
  //    int window_scale = std::stoi(argv[1]);
  //    int cycleDelay = std::stoi(argv[2]);
  //    char const* romFilename = argv[3];

  int window_scale = 10; // std::stoi(argv[1]);
  int cycleDelay = 1;   // std::stoi(argv[2]);
  char const *romFilename = "/home/dosart3/code/c++/chip-8_interpreter/"
                            "cmake-build-debug/Tetris.ch8";

  viewer_t viewer;
  viewer.set_window_title("CHIP-8 Emulator")
      .set_window_scale(window_scale)
      .set_window_width(VIDEO_WIDTH)
      .set_window_height(VIDEO_HEIGHT)
      .set_texture_width(VIDEO_WIDTH)
      .set_texture_height(VIDEO_HEIGHT)
      .build();

  auto chip8 = make_chip8();
  load_rom(chip8.get(), romFilename);

  int video_pitch = sizeof(chip8->video[0]) * VIDEO_WIDTH;

  auto lastCycleTime = std::chrono::high_resolution_clock::now();
  bool quit = false;

  while (!quit) {
    quit = viewer.process_input(chip8->keypad);

    auto currentTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(
                   currentTime - lastCycleTime)
                   .count();

    if (dt > static_cast<float>(cycleDelay)) {
      lastCycleTime = currentTime;

      run_cycle(chip8.get());

      viewer.update(chip8->video, video_pitch);
    }
  }

  return 0;
}