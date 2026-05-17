#include "Assets.h"
#include "Bullet.h"
#include "Camera.h"
#include "CollisionMap.h"
#include "Constants.h"
#include "Effects.h"
#include "Grenade.h"
#include "HudRenderer.h"
#include "Input.h"
#include "MathUtil.h"
#include "Player.h"
#include "Renderer2D.h"
#include "SoundSystem.h"
#include "Weapon.h"
#include "Zombie.h"
#include "ZombieDirector.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <array>
#include <cstdio>

namespace {

enum class GamePhase {
    Title,
    Intro,
    Playing
};

constexpr float kIntroDurationSeconds = 0.75f;

bool init_sdl_window(SDL_Window** window, SDL_Renderer** renderer)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (!TTF_Init()) {
        std::fprintf(stderr, "TTF_Init failed: %s\n", SDL_GetError());
        SDL_Quit();
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
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, nullptr);
    if (*renderer == nullptr) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        TTF_Quit();
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
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    zg::WeaponCatalog weapon_catalog;
    if (!weapon_catalog.load(renderer, "assets/weapons/weapon.ini")) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    zg::CollisionMap collision_map;
    if (!collision_map.load("assets/collision/building1_form.png")) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    zg::Renderer2D renderer2d(renderer);
    zg::HudRenderer hud(renderer);
    zg::SoundSystem sounds;
    if (!sounds.init() || !sounds.load_defaults()) {
        sounds.shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    zg::Player player;
    zg::Camera camera;
    zg::EffectsSystem effects;
    zg::GrenadeSystem grenades;
    zg::InputState input;
    zg::BulletSystem bullets;
    zg::ZombieDirector zombie_director;
    zg::WeaponState weapon;
    if (!weapon.load_default_inventory(weapon_catalog)) {
        sounds.shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    std::array<zg::Zombie, zg::kZombiePoolSize> zombies{};

    GamePhase phase = GamePhase::Title;
    float intro_timer = 0.0f;
    camera.x = zg::kTitleCameraX;
    camera.target_x = zg::kTitleCameraX;
    camera.set_zoom(zg::kTitleCameraZoom);
    zombie_director.init(collision_map);

    bool running = true;
    Uint64 last_ticks = SDL_GetTicks();
    while (running) {
        input.poll(renderer);
        if (input.quit) {
            running = false;
        }
        if (input.switch_slot >= 0) {
            weapon.switch_to_slot(input.switch_slot);
        } else if (input.cycle_weapon != 0) {
            weapon.cycle(input.cycle_weapon);
        }

        const Uint64 now_ticks = SDL_GetTicks();
        const float dt = static_cast<float>(now_ticks - last_ticks) / 1000.0f;
        last_ticks = now_ticks;

        const float aim_world_x = input.mouse_x + camera.x;
        const float aim_world_y = input.mouse_y;

        if (phase == GamePhase::Title) {
            camera.target_x = zg::kTitleCameraX;
            camera.x = zg::kTitleCameraX;
            camera.target_zoom = zg::kTitleCameraZoom;
            if (input.confirm_pressed) {
                phase = GamePhase::Intro;
                intro_timer = 0.0f;
            }
        } else {
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
            if (input.reload_pressed) {
                weapon.start_reload();
            }
            weapon.update(dt);
            if (grenades.try_throw(player, input.grenade_pressed, &effects)) {
                sounds.play(zg::SoundId::GrenadeThrow, 0.8f);
            }
            bool fired = false;
            const zg::WeaponDefinition* current_weapon = weapon.current_definition();
            bullets.try_fire(
                player,
                weapon.can_fire() && input.fire_down && input.mouse_in_view,
                weapon.can_fire() && input.fire_pressed && input.mouse_in_view,
                current_weapon != nullptr && current_weapon->full_auto ? zg::FireMode::FullAuto : zg::FireMode::SemiAuto,
                current_weapon != nullptr ? current_weapon->fire_interval_seconds() : zg::kGlockFireIntervalSeconds,
                dt,
                &effects,
                &fired);
            if (fired) {
                weapon.consume_round();
                current_weapon = weapon.current_definition();
                if (current_weapon != nullptr) {
                    sounds.play_file(current_weapon->shoot_sound_path.c_str(), current_weapon->loudness);
                    camera.add_shake(current_weapon->shake_duration, current_weapon->shake_magnitude);
                }
            }
            bullets.update(dt);
            bullets.resolve_collisions(
                collision_map,
                assets.zombie_mask,
                zombies.data(),
                static_cast<int>(zombies.size()),
                &effects);
            grenades.update(
                collision_map,
                &player,
                zombies.data(),
                static_cast<int>(zombies.size()),
                &effects,
                dt);
            if (grenades.consume_explosion_event()) {
                sounds.play_random_explosion();
                camera.add_shake(zg::kExplosionShakeDuration, zg::kExplosionShakeMagnitude);
            }
            zombie_director.update(collision_map, player, zombies.data(), static_cast<int>(zombies.size()), dt);
            for (int i = 0; i < static_cast<int>(zombies.size()); ++i) {
                zombies[static_cast<size_t>(i)].update(collision_map, zombie_director.move_axis_for(i), dt);
            }
            effects.update(collision_map, dt);

            if (phase == GamePhase::Intro) {
                intro_timer += dt;
                const float t = intro_timer >= kIntroDurationSeconds ? 1.0f : intro_timer / kIntroDurationSeconds;
                const float gameplay_target = zg::clamp_float(player.x - static_cast<float>(zg::kLogicalWidth) * 0.5f, 0.0f, zg::kWorldWidth - zg::kLogicalWidth);
                camera.target_x = gameplay_target;
                camera.x = zg::kTitleCameraX + (gameplay_target - zg::kTitleCameraX) * t;
                camera.target_zoom = zg::kTitleCameraZoom + (zg::kGameplayCameraZoom - zg::kTitleCameraZoom) * t;
                camera.zoom = camera.target_zoom;
                if (t >= 1.0f) {
                    phase = GamePhase::Playing;
                }
            } else {
                camera.target_zoom = zg::kGameplayCameraZoom;
                camera.follow(player.x, input.mouse_x, input.mouse_in_view, dt);
            }
        }
        camera.update(dt);

        float player_alpha = 1.0f;
        if (phase == GamePhase::Title) {
            player_alpha = 0.0f;
        } else if (phase == GamePhase::Intro) {
            const float t = intro_timer / kIntroDurationSeconds;
            if (t <= zg::kIntroPlayerRevealT) {
                player_alpha = 0.0f;
            } else {
                const float fade_t = (t - zg::kIntroPlayerRevealT) / (1.0f - zg::kIntroPlayerRevealT);
                player_alpha = zg::clamp_float(fade_t, 0.0f, 1.0f);
            }
        }

        renderer2d.begin_frame();
        renderer2d.render_scene(
            assets,
            player,
            zombies.data(),
            static_cast<int>(zombies.size()),
            bullets.bullets(),
            bullets.bullet_count(),
            grenades.grenades(),
            grenades.grenade_count(),
            grenades.explosions(),
            grenades.explosion_count(),
            effects,
            weapon.current_definition(),
            phase == GamePhase::Playing || (phase == GamePhase::Intro && intro_timer / kIntroDurationSeconds >= zg::kIntroPlayerRevealT),
            player_alpha,
            camera);
        if (phase == GamePhase::Playing) {
            hud.render_weapon_status(assets.bullet_icon, player, weapon.current_definition(), weapon, camera);
            hud.render_top_bar(weapon, zombie_director.wave(), zombie_director.alive_count());
        }
        const float ui_alpha = phase == GamePhase::Title ? 1.0f : (phase == GamePhase::Intro ? 1.0f - (intro_timer / kIntroDurationSeconds) : 0.0f);
        hud.render_title_screen(ui_alpha, phase == GamePhase::Title);
        renderer2d.end_frame();
    }

    sounds.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
