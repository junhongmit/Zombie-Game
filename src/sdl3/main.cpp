#include "Assets.h"
#include "Bullet.h"
#include "Camera.h"
#include "CollisionMap.h"
#include "Constants.h"
#include "Input.h"
#include "Player.h"
#include "Renderer2D.h"
#include "Zombie.h"

#include <SDL3/SDL.h>

#include <array>
#include <cstdio>

namespace {

bool init_sdl_window(SDL_Window** window, SDL_Renderer** renderer)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow(
        "ZombieGame SDL3 Prototype",
        960,
        720,
        SDL_WINDOW_RESIZABLE
    );
    if (*window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, nullptr);
    if (*renderer == nullptr) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderVSync(*renderer, 1);
    SDL_SetRenderLogicalPresentation(
        *renderer,
        zg::kLogicalWidth,
        zg::kLogicalHeight,
        SDL_LOGICAL_PRESENTATION_LETTERBOX
    );

    return true;
}

} // namespace

int main(int, char**)
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!init_sdl_window(&window, &renderer)) {
        return 1;
    }

    zg::Assets assets;
    if (!assets.load(renderer)) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    zg::CollisionMap collision_map;
    if (!collision_map.load("image/building1 form.png")) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    zg::Renderer2D renderer2d(renderer);
    zg::Player player;
    zg::Camera camera;
    zg::InputState input;
    zg::BulletSystem bullets;
    std::array<zg::Zombie, 6> zombies = {{
        zg::Zombie(36.0f, 400.0f, 16.0f, true, 0),
        zg::Zombie(160.0f, 400.0f, 22.0f, true, 3),
        zg::Zombie(330.0f, 400.0f, 18.0f, false, 8),
        zg::Zombie(690.0f, 400.0f, 20.0f, false, 12),
        zg::Zombie(835.0f, 400.0f, 24.0f, true, 17),
        zg::Zombie(1015.0f, 400.0f, 15.0f, false, 21),
    }};

    camera.follow(player.x);

    bool running = true;
    Uint64 last_ticks = SDL_GetTicks();
    while (running) {
        input.poll(renderer);
        if (input.quit) {
            running = false;
        }

        const Uint64 now_ticks = SDL_GetTicks();
        const float dt = static_cast<float>(now_ticks - last_ticks) / 1000.0f;
        last_ticks = now_ticks;

        const float aim_world_x = input.mouse_x + camera.x;
        const float aim_world_y = input.mouse_y;
        player.update(
            collision_map,
            input.move_axis,
            input.jump_pressed,
            aim_world_x,
            aim_world_y,
            input.mouse_in_view,
            dt);
        if (input.stair_pressed) {
            player.try_use_stairs(collision_map);
        }
        bullets.try_fire(
            player,
            input.fire_down && input.mouse_in_view,
            input.fire_pressed && input.mouse_in_view,
            zg::FireMode::SemiAuto,
            dt);
        bullets.update(dt);
        bullets.resolve_collisions(
            collision_map,
            assets.zombie_mask,
            zombies.data(),
            static_cast<int>(zombies.size()));
        for (zg::Zombie& zombie : zombies) {
            zombie.update(collision_map, dt);
        }
        camera.follow(player.x);

        renderer2d.begin_frame();
        renderer2d.render_scene(
            assets,
            player,
            zombies.data(),
            static_cast<int>(zombies.size()),
            bullets.bullets(),
            bullets.bullet_count(),
            camera);
        renderer2d.end_frame();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
