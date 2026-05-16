#include "Renderer2D.h"

#include "Assets.h"
#include "Bullet.h"
#include "Camera.h"
#include "Constants.h"
#include "MathUtil.h"
#include "Player.h"
#include "Texture.h"
#include "Zombie.h"

#include <algorithm>
#include <cmath>

namespace zg {

Renderer2D::Renderer2D(SDL_Renderer* renderer)
    : renderer_(renderer)
{
}

void Renderer2D::begin_frame()
{
    SDL_SetRenderDrawColor(renderer_, 11, 18, 26, 255);
    SDL_RenderClear(renderer_);
}

void Renderer2D::render_scene(
    const Assets& assets,
    const Player& player,
    const Zombie* zombies,
    int zombie_count,
    const Bullet* bullets,
    int bullet_count,
    const Camera& camera)
{
    render_fullscreen(assets.sky);
    render_scrolling_layer(assets.backcity3, camera.x, 0.2f, 316.0f, 116.0f);
    render_scrolling_layer(assets.backcity2, camera.x, 0.25f, 316.0f, 116.0f);
    render_scrolling_layer(assets.backcity1, camera.x, 0.5f, 316.0f, 116.0f);
    render_world_layer(assets.building, camera.x);

    for (int i = 0; i < zombie_count; ++i) {
        render_zombie(assets.zombie, zombies[i], camera);
    }
    for (int i = 0; i < bullet_count; ++i) {
        render_bullet(bullets[i], camera);
    }
    render_player(assets.hero, player, camera);
    render_weapon(assets.weapon_glock, player, camera);
}

void Renderer2D::end_frame()
{
    SDL_RenderPresent(renderer_);
}

void Renderer2D::render_fullscreen(const Texture& texture)
{
    const SDL_FRect dst{0.0f, 0.0f, static_cast<float>(kLogicalWidth), static_cast<float>(kLogicalHeight)};
    SDL_RenderTexture(renderer_, texture.get(), nullptr, &dst);
}

void Renderer2D::render_scrolling_layer(
    const Texture& texture,
    float camera_x,
    float parallax,
    float y,
    float height)
{
    const float view_width = static_cast<float>(kLogicalWidth);
    const float max_source_x = std::max(0.0f, texture.width() - view_width);
    const float source_x = clamp_float(camera_x * parallax, 0.0f, max_source_x);
    const SDL_FRect src{source_x, 0.0f, std::min(view_width, texture.width() - source_x), texture.height()};
    const SDL_FRect dst{0.0f, y, view_width, height};
    SDL_RenderTexture(renderer_, texture.get(), &src, &dst);
}

void Renderer2D::render_world_layer(const Texture& texture, float camera_x)
{
    const float view_width = static_cast<float>(kLogicalWidth);
    const float max_source_x = std::max(0.0f, texture.width() - view_width);
    const float source_x = clamp_float(camera_x, 0.0f, max_source_x);
    const SDL_FRect src{source_x, 0.0f, std::min(view_width, texture.width() - source_x), texture.height()};
    const SDL_FRect dst{0.0f, 0.0f, view_width, static_cast<float>(kLogicalHeight)};
    SDL_RenderTexture(renderer_, texture.get(), &src, &dst);
}

void Renderer2D::render_player(const Texture& hero, const Player& player, const Camera& camera)
{
    SDL_FRect src{static_cast<float>(player.walk_frame * 17), 0.0f, 18.0f, 33.0f};
    SDL_FRect dst{std::round(player.x - camera.x), player.y, 18.0f, 33.0f};
    const SDL_FPoint center{9.0f, 16.5f};
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderTextureRotated(renderer_, hero.get(), &src, &dst, 0.0, &center, flip);
}

void Renderer2D::render_weapon(const Texture& weapon, const Player& player, const Camera& camera)
{
    const float frame_width = weapon.width() * 0.5f;
    const float frame_height = weapon.height();
    const SDL_FRect src{0.0f, 0.0f, frame_width, frame_height};

    const float pivot_screen_x = std::round(player.x - camera.x + (player.facing_right ? 8.0f : 10.0f));
    const float pivot_screen_y = std::round(player.y + 13.0f);
    const SDL_FPoint pivot{
        player.facing_right ? 4.0f : frame_width - 4.0f,
        3.0f
    };
    const SDL_FRect dst{pivot_screen_x - pivot.x, pivot_screen_y - pivot.y, frame_width, frame_height};
    const double aim_degrees = static_cast<double>(player.aim_angle_radians * 180.0f / 3.1415926535f);
    const double angle_degrees = player.facing_right ? aim_degrees : 180.0 + aim_degrees;
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderTextureRotated(renderer_, weapon.get(), &src, &dst, angle_degrees, &pivot, flip);
}

void Renderer2D::render_bullet(const Bullet& bullet, const Camera& camera)
{
    if (!bullet.active) {
        return;
    }

    const float start_x = std::round(bullet.x - camera.x);
    const float start_y = std::round(bullet.y);
    const float length = 18.0f;
    const float speed = std::sqrt(bullet.vx * bullet.vx + bullet.vy * bullet.vy);
    if (speed <= 0.0f) {
        return;
    }

    const float end_x = start_x - bullet.vx / speed * length;
    const float end_y = start_y - bullet.vy / speed * length;

    SDL_SetRenderDrawColor(renderer_, 248, 226, 121, 255);
    SDL_RenderLine(renderer_, start_x, start_y, end_x, end_y);
}

void Renderer2D::render_zombie(const Texture& texture, const Zombie& zombie, const Camera& camera)
{
    if (!zombie.active) {
        return;
    }

    if (zombie.hit_flash > 0.0f) {
        SDL_SetTextureColorMod(texture.get(), 255, 90, 90);
    }

    if (zombie.alive) {
        SDL_FRect src{static_cast<float>(zombie.walk_frame * 17), 0.0f, kZombieWidth, kZombieHeight};
        SDL_FRect dst{std::round(zombie.x - camera.x), zombie.y, kZombieWidth, kZombieHeight};
        const SDL_FPoint center{9.0f, 16.0f};
        const SDL_FlipMode flip = zombie.walking_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, 0.0, &center, flip);
    } else {
        const SDL_FRect src{477.0f, 0.0f, 17.0f, 32.0f};
        const SDL_FRect dst{
            std::round(zombie.x - camera.x),
            zombie.y + kZombieCorpseYOffset,
            kZombieCorpseWidth,
            kZombieCorpseHeight
        };
        const SDL_FPoint center{kZombieCorpseWidth * 0.5f, kZombieCorpseHeight * 0.5f};
        SDL_SetTextureAlphaMod(texture.get(), static_cast<Uint8>(std::round(zombie.corpse_alpha)));
        SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, zombie.corpse_angle_degrees, &center, SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(texture.get(), 255);
    }

    if (zombie.hit_flash > 0.0f) {
        SDL_SetTextureColorMod(texture.get(), 255, 255, 255);
    }
}

} // namespace zg
