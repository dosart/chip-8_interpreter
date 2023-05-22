#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <cstdint>

#ifndef CHIP8_INTERPRETER_PLATFORM_H
#define CHIP8_INTERPRETER_PLATFORM_H

class viewer_t {
public:
  viewer_t() = default;
  ~viewer_t();

  void build();
  void update(void const *buffer, int pitch);
  bool process_input(uint8_t *chip8_keypad);
  void delay(uint32_t);

  viewer_t &set_window_title(char const *title);
  viewer_t &set_window_width(int width);
  viewer_t &set_window_height(int height);
  viewer_t &set_texture_width(int width);
  viewer_t &set_texture_height(int height);
  viewer_t &set_window_scale(int scale);

private:
  SDL_Window *window{};
  SDL_Renderer *renderer{};
  SDL_Texture *texture{};

  int window_width{};
  int window_height{};
  int window_scale = 1;
  char const *window_title{};

  int texture_width{};
  int texture_height{};
};

#endif // CHIP8_INTERPRETER_PLATFORM_H
