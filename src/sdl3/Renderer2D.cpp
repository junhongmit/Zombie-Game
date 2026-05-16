#include "Renderer2D.h"

#include "Assets.h"
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
    render_aim_line(player, camera);
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

void Renderer2D::render_aim_line(const Player& player, const Camera& camera)
{
    const float pivot_x = std::round(player.x - camera.x + 9.0f);
    const float pivot_y = std::round(player.y + 13.0f);
    const float aim_x = std::round(player.aim_world_x - camera.x);
    const float aim_y = std::round(player.aim_world_y);

    SDL_SetRenderDrawColor(renderer_, 222, 218, 185, 160);
    SDL_RenderLine(renderer_, pivot_x, pivot_y, aim_x, aim_y);
    SDL_RenderLine(renderer_, aim_x - 4.0f, aim_y, aim_x + 4.0f, aim_y);
    SDL_RenderLine(renderer_, aim_x, aim_y - 4.0f, aim_x, aim_y + 4.0f);
}

void Renderer2D::render_zombie(const Texture& texture, const Zombie& zombie, const Camera& camera)
{
    SDL_FRect src{static_cast<float>(zombie.walk_frame * 17), 0.0f, 18.0f, 32.0f};
    SDL_FRect dst{std::round(zombie.x - camera.x), zombie.y, 18.0f, 32.0f};
    const SDL_FPoint center{9.0f, 16.0f};
    const SDL_FlipMode flip = zombie.walking_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, 0.0, &center, flip);
}

} // namespace zg
