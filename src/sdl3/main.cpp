#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr int kLogicalWidth = 640;
constexpr int kLogicalHeight = 480;

struct Texture {
    SDL_Texture* handle = nullptr;
    float width = 0.0f;
    float height = 0.0f;
};

struct Sprite {
    Texture* texture = nullptr;
    SDL_FRect dst{};

    Sprite(Texture* texture_in, SDL_FRect dst_in)
        : texture(texture_in), dst(dst_in)
    {
    }
};

void destroy_texture(Texture& texture)
{
    if (texture.handle != nullptr) {
        SDL_DestroyTexture(texture.handle);
        texture.handle = nullptr;
    }
}

bool load_texture(SDL_Renderer* renderer, const char* path, Texture& texture)
{
    texture.handle = IMG_LoadTexture(renderer, path);
    if (texture.handle == nullptr) {
        std::fprintf(stderr, "Failed to load %s: %s\n", path, SDL_GetError());
        return false;
    }

    if (!SDL_GetTextureSize(texture.handle, &texture.width, &texture.height)) {
        std::fprintf(stderr, "Failed to query %s: %s\n", path, SDL_GetError());
        destroy_texture(texture);
        return false;
    }

    SDL_SetTextureScaleMode(texture.handle, SDL_SCALEMODE_NEAREST);
    return true;
}

void render_texture(SDL_Renderer* renderer, const Texture& texture, const SDL_FRect& dst)
{
    SDL_RenderTexture(renderer, texture.handle, nullptr, &dst);
}

} // namespace

int main(int, char**)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "ZombieGame SDL3 Prototype",
        960,
        720,
        SDL_WINDOW_RESIZABLE
    );
    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(
        renderer,
        kLogicalWidth,
        kLogicalHeight,
        SDL_LOGICAL_PRESENTATION_LETTERBOX
    );

    Texture sky;
    Texture backcity1;
    Texture backcity2;
    Texture backcity3;
    Texture building;
    Texture hero;

    const bool loaded =
        load_texture(renderer, "image/sky1.png", sky) &&
        load_texture(renderer, "image/backcity1.png", backcity1) &&
        load_texture(renderer, "image/backcity2.png", backcity2) &&
        load_texture(renderer, "image/backcity3.png", backcity3) &&
        load_texture(renderer, "image/building1.png", building) &&
        load_texture(renderer, "image/man1.png", hero);

    if (!loaded) {
        destroy_texture(hero);
        destroy_texture(building);
        destroy_texture(backcity3);
        destroy_texture(backcity2);
        destroy_texture(backcity1);
        destroy_texture(sky);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::vector<Sprite> scene = {
        {&sky, {0.0f, 0.0f, 640.0f, 480.0f}},
        {&backcity3, {0.0f, 316.0f, 640.0f, 116.0f}},
        {&backcity2, {0.0f, 316.0f, 640.0f, 116.0f}},
        {&backcity1, {0.0f, 316.0f, 640.0f, 116.0f}},
        {&building, {0.0f, 0.0f, 640.0f, 480.0f}},
        {&hero, {302.0f, 399.0f, 18.0f, 33.0f}},
    };

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 11, 18, 26, 255);
        SDL_RenderClear(renderer);

        for (const Sprite& sprite : scene) {
            render_texture(renderer, *sprite.texture, sprite.dst);
        }

        SDL_RenderPresent(renderer);
    }

    destroy_texture(hero);
    destroy_texture(building);
    destroy_texture(backcity3);
    destroy_texture(backcity2);
    destroy_texture(backcity1);
    destroy_texture(sky);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
