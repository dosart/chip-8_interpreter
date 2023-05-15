#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <cstdint>

#ifndef CHIP8_INTERPRETER_PLATFORM_H
#define CHIP8_INTERPRETER_PLATFORM_H


class platform_t {
public:
    platform_t(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
    ~platform_t();

    void update(void const* buffer, int pitch);
    bool process_input(uint8_t* keys);

private:
    SDL_Window* window{};
    SDL_GLContext gl_context{};
    GLuint framebuffer_texture;
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};
};


#endif //CHIP8_INTERPRETER_PLATFORM_H
