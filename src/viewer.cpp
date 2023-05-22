#include "viewer.h"
#include <stdexcept>

void viewer_t::build() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw std::runtime_error(SDL_GetError());
  }

  window = SDL_CreateWindow(window_title, 0, 0, window_width * window_scale,
                            window_height * window_scale, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    throw std::runtime_error(SDL_GetError());
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    SDL_DestroyWindow(window);
    throw std::runtime_error(SDL_GetError());
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_STREAMING, texture_width,
                              texture_height);
  if (texture == nullptr) {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    throw std::runtime_error(SDL_GetError());
  }
}

viewer_t::~viewer_t() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void viewer_t::update(void const *buffer, int pitch) {
  // Fetch the new texture
  SDL_UpdateTexture(texture, nullptr, buffer, pitch);
  // Clear the renderer
  SDL_RenderClear(renderer);
  // Copy the new texture to the renderer
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  // Apply the texture to the renderer
  SDL_RenderPresent(renderer);
}

bool viewer_t::process_input(uint8_t *chip8_keypad) {
  bool quit = false;

  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      quit = true;
    } break;

    case SDL_KEYDOWN: {
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE: {
        quit = true;
      } break;

      case SDLK_x: {
        chip8_keypad[0] = 1;
      } break;

      case SDLK_1: {
        chip8_keypad[1] = 1;
      } break;

      case SDLK_2: {
        chip8_keypad[2] = 1;
      } break;

      case SDLK_3: {
        chip8_keypad[3] = 1;
      } break;

      case SDLK_q: {
        chip8_keypad[4] = 1;
      } break;

      case SDLK_w: {
        chip8_keypad[5] = 1;
      } break;

      case SDLK_e: {
        chip8_keypad[6] = 1;
      } break;

      case SDLK_a: {
        chip8_keypad[7] = 1;
      } break;

      case SDLK_s: {
        chip8_keypad[8] = 1;
      } break;

      case SDLK_d: {
        chip8_keypad[9] = 1;
      } break;

      case SDLK_z: {
        chip8_keypad[0xA] = 1;
      } break;

      case SDLK_c: {
        chip8_keypad[0xB] = 1;
      } break;

      case SDLK_4: {
        chip8_keypad[0xC] = 1;
      } break;

      case SDLK_r: {
        chip8_keypad[0xD] = 1;
      } break;

      case SDLK_f: {
        chip8_keypad[0xE] = 1;
      } break;

      case SDLK_v: {
        chip8_keypad[0xF] = 1;
      } break;
      }
    } break;

    case SDL_KEYUP: {
      switch (event.key.keysym.sym) {
      case SDLK_x: {
        chip8_keypad[0] = 0;
      } break;

      case SDLK_1: {
        chip8_keypad[1] = 0;
      } break;

      case SDLK_2: {
        chip8_keypad[2] = 0;
      } break;

      case SDLK_3: {
        chip8_keypad[3] = 0;
      } break;

      case SDLK_q: {
        chip8_keypad[4] = 0;
      } break;

      case SDLK_w: {
        chip8_keypad[5] = 0;
      } break;

      case SDLK_e: {
        chip8_keypad[6] = 0;
      } break;

      case SDLK_a: {
        chip8_keypad[7] = 0;
      } break;

      case SDLK_s: {
        chip8_keypad[8] = 0;
      } break;

      case SDLK_d: {
        chip8_keypad[9] = 0;
      } break;

      case SDLK_z: {
        chip8_keypad[0xA] = 0;
      } break;

      case SDLK_c: {
        chip8_keypad[0xB] = 0;
      } break;

      case SDLK_4: {
        chip8_keypad[0xC] = 0;
      } break;

      case SDLK_r: {
        chip8_keypad[0xD] = 0;
      } break;

      case SDLK_f: {
        chip8_keypad[0xE] = 0;
      } break;

      case SDLK_v: {
        chip8_keypad[0xF] = 0;
      } break;
      }
    } break;
    }
  }

  return quit;
}

viewer_t &viewer_t::set_window_width(int width) {
  window_width = width;
  return *this;
}

viewer_t &viewer_t::set_window_height(int height) {
  window_height = height;
  return *this;
}

viewer_t &viewer_t::set_texture_width(int width) {
  texture_width = width;
  return *this;
}

viewer_t &viewer_t::set_texture_height(int height) {
  texture_height = height;
  return *this;
}

viewer_t &viewer_t::set_window_title(const char *title) {
  window_title = title;
  return *this;
}

viewer_t &viewer_t::set_window_scale(int scale) {
  window_scale = scale;
  return *this;
}
void viewer_t::delay(uint32_t delay) {
  SDL_Delay(delay);
}
