#include "chip8.h"
#include "viewer.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <Scale> <ROM>\n";
    std::exit(EXIT_FAILURE);
  }

  int window_scale = std::stoi(argv[1]);
  char const *romFilename = argv[3];

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
  bool quit = false;

  uint32_t speed = 3;
  while (!quit) {
    quit = viewer.process_input(chip8->keypad);
    run_cycle(chip8.get());
    viewer.update(chip8->video, video_pitch);
    viewer.delay(speed);
  }

  return 0;
}