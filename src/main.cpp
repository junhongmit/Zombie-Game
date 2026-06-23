#include "Assets.h"
#include "Bullet.h"
#include "Camera.h"
#include "CollisionMap.h"
#include "Constants.h"
#include "Effects.h"
#include "Grenade.h"
#include "HudRenderer.h"
#include "Input.h"
#include "InventoryState.h"
#include "MathUtil.h"
#include "Player.h"
#include "Renderer2D.h"
#include "SoundSystem.h"
#include "Weapon.h"
#include "Zombie.h"
#include "ZombieDirector.h"
#include "ui/WorkbenchScreen.h"
#include "ui/InventoryScreen.h"
#include "ui/MarketScreen.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <array>
#include <cstdio>

namespace {

void show_startup_error(SDL_Window* window, const char* message)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Zombie Game - Startup Error", message, window);
}

enum class GamePhase {
    Title,
    Intro,
    Workbench,
    Market,
    Inventory,
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
        zg::kWindowDefaultWidth,
        zg::kWindowDefaultHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
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

    SDL_SetWindowMinimumSize(*window, zg::kWindowMinWidth, zg::kWindowMinHeight);
    SDL_SetWindowAspectRatio(*window, 16.0f / 9.0f, 16.0f / 9.0f);

    SDL_SetRenderVSync(*renderer, 1);
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
        show_startup_error(window, "Failed to load startup assets. Check assets/scenes, assets/ui, and related metadata paths.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    zg::WeaponCatalog weapon_catalog;
    if (!weapon_catalog.load(renderer, "assets/weapons/weapons.json") &&
        !weapon_catalog.load(renderer, "assets/weapons/weapon.ini")) {
        show_startup_error(window, "Failed to load weapon metadata from assets/weapons/weapons.json or assets/weapons/weapon.ini.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    zg::CollisionMap collision_map;
    if (!collision_map.load("assets/collision/building1_form.png")) {
        show_startup_error(window, "Failed to load the collision map from assets/collision/building1_form.png.");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    zg::Renderer2D renderer2d(renderer);
    zg::HudRenderer hud(renderer);
    zg::WorkbenchScreen workbench(renderer);
    zg::InventoryScreen inventory_screen(renderer);
    zg::MarketScreen market_screen(renderer);
    zg::SoundSystem sounds;
    if (!sounds.init() || !sounds.load_defaults()) {
        show_startup_error(window, "Failed to initialize or load audio assets.");
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
    zg::InventoryState inventory;
    if (!weapon.load_default_inventory(weapon_catalog)) {
        show_startup_error(window, "Failed to build the default weapon inventory from weapon metadata.");
        sounds.shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    inventory.load_demo(renderer);
    std::array<zg::Zombie, zg::kZombiePoolSize> zombies{};

    GamePhase phase = GamePhase::Title;
    zg::TitleMenuAction armed_title_action = zg::TitleMenuAction::None;
    float intro_timer = 0.0f;
    camera.x = zg::kTitleCameraX;
    camera.target_x = zg::kTitleCameraX;
    camera.set_zoom(zg::kTitleCameraZoom);
    zombie_director.init(collision_map);

    bool running = true;
    Uint64 last_ticks = SDL_GetTicks();
    while (running) {
        weapon_catalog.update_relight_jobs(renderer);
        input.poll(renderer);
        inventory.sync_from_weapon_state(weapon);
        if (input.switch_mode >= 0) {
            inventory.set_current_mode_by_index(input.switch_mode);
        }
        if (input.quit) {
            running = false;
        }
        if (input.back_pressed) {
            if (phase == GamePhase::Workbench) {
                phase = GamePhase::Title;
                armed_title_action = zg::TitleMenuAction::None;
            } else if (phase == GamePhase::Market) {
                phase = GamePhase::Title;
                armed_title_action = zg::TitleMenuAction::None;
            } else if (phase == GamePhase::Inventory) {
                phase = GamePhase::Playing;
            } else if (phase == GamePhase::Title) {
                running = false;
            } else if (phase == GamePhase::Playing) {
                running = false;
            }
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
            if (input.fire_pressed) {
                armed_title_action = hud.hit_test_title_menu(input.ui_mouse_x, input.ui_mouse_y, input.ui_mouse_in_view);
            }
            zg::TitleMenuAction title_action = zg::TitleMenuAction::None;
            if (input.fire_released) {
                const zg::TitleMenuAction released_action =
                    hud.hit_test_title_menu(input.ui_mouse_x, input.ui_mouse_y, input.ui_mouse_in_view);
                if (armed_title_action != zg::TitleMenuAction::None &&
                    armed_title_action == released_action) {
                    title_action = released_action;
                }
                armed_title_action = zg::TitleMenuAction::None;
            }
            if (title_action == zg::TitleMenuAction::Exit) {
                running = false;
            } else if (title_action == zg::TitleMenuAction::Loadout) {
                phase = GamePhase::Workbench;
            } else if (title_action == zg::TitleMenuAction::Market) {
                phase = GamePhase::Market;
            } else if (input.confirm_pressed || title_action == zg::TitleMenuAction::Start) {
                phase = GamePhase::Intro;
                intro_timer = 0.0f;
                armed_title_action = zg::TitleMenuAction::None;
            }
        } else if (phase == GamePhase::Workbench) {
            camera.target_x = zg::kTitleCameraX;
            camera.x = zg::kTitleCameraX;
            camera.target_zoom = zg::kTitleCameraZoom;
        } else if (phase == GamePhase::Market) {
            camera.target_x = zg::kTitleCameraX;
            camera.x = zg::kTitleCameraX;
            camera.target_zoom = zg::kTitleCameraZoom;
        } else if (phase == GamePhase::Inventory) {
            if (input.inventory_pressed) {
                phase = GamePhase::Playing;
            }
        } else {
            if (input.inventory_pressed) {
                phase = GamePhase::Inventory;
            }
            if (input.switch_slot >= 0) {
                weapon.switch_to_slot(input.switch_slot);
            } else if (input.cycle_weapon != 0) {
                weapon.cycle(input.cycle_weapon);
            }

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
                const float gameplay_target = zg::clamp_float(player.x - zg::kGameplayViewWidth * 0.5f, 0.0f, zg::kWorldWidth - zg::kGameplayViewWidth);
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
        if (phase == GamePhase::Title || phase == GamePhase::Workbench || phase == GamePhase::Market) {
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
        const SDL_FRect presentation_rect = renderer2d.presentation_rect();
        const SDL_FRect ui_presentation_rect{
            0.0f,
            0.0f,
            static_cast<float>(zg::kInternalRenderWidth),
            static_cast<float>(zg::kInternalRenderHeight)
        };
        if (phase == GamePhase::Workbench) {
            const bool close_workbench = workbench.render(
                assets,
                weapon,
                ui_presentation_rect,
                dt,
                input.wheel_x,
                input.wheel_y,
                input.ui_mouse_x,
                input.ui_mouse_y,
                input.ui_mouse_in_view,
                input.fire_down,
                input.fire_pressed,
                input.fire_released);
            if (close_workbench) {
                phase = GamePhase::Title;
                armed_title_action = zg::TitleMenuAction::None;
            }
        } else if (phase == GamePhase::Market) {
            const bool close_market = market_screen.render(
                assets,
                inventory,
                weapon_catalog,
                ui_presentation_rect,
                dt,
                input.ui_mouse_x,
                input.ui_mouse_y,
                input.ui_mouse_in_view,
                input.fire_down,
                input.fire_pressed,
                input.fire_released);
            if (close_market) {
                phase = GamePhase::Title;
                armed_title_action = zg::TitleMenuAction::None;
            }
        } else {
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
            if (phase == GamePhase::Playing || phase == GamePhase::Inventory) {
                hud.render_weapon_status(assets.bullet_icon, player, weapon.current_definition(), weapon, camera, presentation_rect);
                hud.render_gameplay_hud(
                    assets,
                    player,
                    inventory,
                    weapon,
                    zombie_director.wave(),
                    zombie_director.alive_count(),
                    ui_presentation_rect);
            }
            if (phase == GamePhase::Inventory) {
                const bool close_inventory = inventory_screen.update_and_render(
                    assets,
                    inventory,
                    weapon,
                    ui_presentation_rect,
                    dt,
                    input.ui_mouse_x,
                    input.ui_mouse_y,
                    input.ui_mouse_in_view,
                    input.fire_down,
                    input.fire_pressed,
                    input.fire_released,
                    input.reload_pressed,
                    input.use_pressed,
                    input.drop_pressed,
                    input.split_pressed);
                if (close_inventory) {
                    phase = GamePhase::Playing;
                }
            }
            const float ui_alpha = phase == GamePhase::Title ? 1.0f : (phase == GamePhase::Intro ? 1.0f - (intro_timer / kIntroDurationSeconds) : 0.0f);
            hud.render_title_screen(
                assets.title_button_skin,
                ui_alpha,
                input.ui_mouse_x,
                input.ui_mouse_y,
                input.ui_mouse_in_view,
                input.fire_down,
                armed_title_action,
                ui_presentation_rect);
        }
        renderer2d.end_frame();
    }

    sounds.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
