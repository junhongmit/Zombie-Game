#pragma once

#include <SDL3/SDL.h>

namespace zg {

class Texture {
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool load(SDL_Renderer* renderer, const char* path, bool use_white_color_key = false);
    bool load_from_surface(SDL_Renderer* renderer, SDL_Surface* surface);
    void reset();

    SDL_Texture* get() const;
    float width() const;
    float height() const;
    bool valid() const;

private:
    SDL_Texture* handle_;
    float width_;
    float height_;
};

} // namespace zg
