#include "Texture.h"
#include "AssetPaths.h"

#include <SDL3_image/SDL_image.h>

#include <cstdio>

namespace zg {

Texture::Texture()
    : handle_(nullptr), width_(0.0f), height_(0.0f)
{
}

Texture::~Texture()
{
    reset();
}

Texture::Texture(Texture&& other) noexcept
    : handle_(other.handle_), width_(other.width_), height_(other.height_)
{
    other.handle_ = nullptr;
    other.width_ = 0.0f;
    other.height_ = 0.0f;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        reset();
        handle_ = other.handle_;
        width_ = other.width_;
        height_ = other.height_;
        other.handle_ = nullptr;
        other.width_ = 0.0f;
        other.height_ = 0.0f;
    }
    return *this;
}

bool Texture::load(SDL_Renderer* renderer, const char* path, bool use_white_color_key)
{
    reset();

    const std::string resolved = resolve_asset_path(path);
    SDL_Surface* surface = IMG_Load(resolved.c_str());
    if (surface == nullptr) {
        std::fprintf(stderr, "Failed to load %s: %s\n", resolved.c_str(), SDL_GetError());
        return false;
    }

    if (use_white_color_key) {
        const Uint32 white = SDL_MapSurfaceRGB(surface, 255, 255, 255);
        SDL_SetSurfaceColorKey(surface, true, white);
    }

    handle_ = SDL_CreateTextureFromSurface(renderer, surface);
    width_ = static_cast<float>(surface->w);
    height_ = static_cast<float>(surface->h);
    SDL_DestroySurface(surface);

    if (handle_ == nullptr) {
        std::fprintf(stderr, "Failed to create texture for %s: %s\n", resolved.c_str(), SDL_GetError());
        width_ = 0.0f;
        height_ = 0.0f;
        return false;
    }

    SDL_SetTextureScaleMode(handle_, SDL_SCALEMODE_NEAREST);
    return true;
}

bool Texture::load_from_surface(SDL_Renderer* renderer, SDL_Surface* surface)
{
    reset();
    if (renderer == nullptr || surface == nullptr) {
        return false;
    }

    handle_ = SDL_CreateTextureFromSurface(renderer, surface);
    width_ = static_cast<float>(surface->w);
    height_ = static_cast<float>(surface->h);
    if (handle_ == nullptr) {
        std::fprintf(stderr, "Failed to create texture from surface: %s\n", SDL_GetError());
        width_ = 0.0f;
        height_ = 0.0f;
        return false;
    }

    SDL_SetTextureScaleMode(handle_, SDL_SCALEMODE_NEAREST);
    return true;
}

void Texture::reset()
{
    if (handle_ != nullptr) {
        SDL_DestroyTexture(handle_);
        handle_ = nullptr;
    }
    width_ = 0.0f;
    height_ = 0.0f;
}

SDL_Texture* Texture::get() const
{
    return handle_;
}

float Texture::width() const
{
    return width_;
}

float Texture::height() const
{
    return height_;
}

bool Texture::valid() const
{
    return handle_ != nullptr;
}

} // namespace zg
