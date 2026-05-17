#include "Renderer2D.h"

#include "Assets.h"
#include "Bullet.h"
#include "Camera.h"
#include "Constants.h"
#include "Effects.h"
#include "Grenade.h"
#include "MathUtil.h"
#include "Player.h"
#include "SpriteCatalog.h"
#include "Texture.h"
#include "Weapon.h"
#include "Zombie.h"

#include <algorithm>
#include <cmath>

namespace zg {

namespace {

float world_zoom(const Camera& camera)
{
    return camera.render_zoom();
}

float world_origin_y(const Camera& camera)
{
    return static_cast<float>(kLogicalHeight) - static_cast<float>(kLogicalHeight) * world_zoom(camera);
}

float world_to_screen_x(float world_x, const Camera& camera)
{
    return (world_x - camera.render_x()) * world_zoom(camera);
}

float world_to_screen_y(float world_y, const Camera& camera)
{
    return world_y * world_zoom(camera) + world_origin_y(camera) + camera.render_y();
}

float scale_world_length(float value, const Camera& camera)
{
    return value * world_zoom(camera);
}

} // namespace

Renderer2D::Renderer2D(SDL_Renderer* renderer)
    : renderer_(renderer)
{
}

Renderer2D::~Renderer2D()
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
    const Grenade* grenades,
    int grenade_count,
    const Explosion* explosions,
    int explosion_count,
    const EffectsSystem& effects,
    const WeaponDefinition* weapon_definition,
    bool show_player,
    float player_alpha,
    const Camera& camera)
{
    render_fullscreen(assets.sky);
    render_scrolling_layer(assets.backcity3, camera, 0.2f, 316.0f, 116.0f);
    render_scrolling_layer(assets.backcity2, camera, 0.25f, 316.0f, 116.0f);
    render_scrolling_layer(assets.backcity1, camera, 0.5f, 316.0f, 116.0f);
    render_world_layer(assets.building, camera);

    for (int i = 0; i < effects.blood_particle_count(); ++i) {
        const BloodParticle& particle = effects.blood_particles()[i];
        if (particle.active) {
            render_blood_particle(particle.x, particle.y, camera);
        }
    }

    for (int i = 0; i < zombie_count; ++i) {
        render_zombie(assets.zombie, zombies[i], camera);
    }
    for (int i = 0; i < bullet_count; ++i) {
        render_bullet(bullets[i], camera);
    }
    for (int i = 0; i < grenade_count; ++i) {
        render_grenade(grenades[i], camera);
    }
    for (int i = 0; i < explosion_count; ++i) {
        render_explosion(assets, explosions[i], camera);
    }
    if (show_player) {
        render_player(assets.hero, player, player_alpha, camera);
    }
    if (show_player && weapon_definition != nullptr) {
        render_weapon(weapon_definition->texture, *weapon_definition, player, player_alpha, camera);
    }

    for (int i = 0; i < effects.smoke_particle_count(); ++i) {
        const SmokeParticle& particle = effects.smoke_particles()[i];
        if (!particle.active) {
            continue;
        }
        const float t = particle.age / particle.lifetime;
        const float size = 10.0f + 18.0f * t;
        const float alpha = 1.0f - t;
        render_smoke_particle(assets.smoke, particle.x, particle.y, size, alpha, camera);
    }
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
    const Camera& camera,
    float parallax,
    float y,
    float height)
{
    const float zoom = world_zoom(camera);
    const float view_width = static_cast<float>(kLogicalWidth) / zoom;
    const float max_source_x = std::max(0.0f, texture.width() - view_width);
    const float source_x = clamp_float(camera.render_x() * parallax, 0.0f, max_source_x);
    const SDL_FRect src{source_x, 0.0f, std::min(view_width, texture.width() - source_x), texture.height()};
    const SDL_FRect dst{
        0.0f,
        world_to_screen_y(y, camera),
        static_cast<float>(kLogicalWidth),
        scale_world_length(height, camera)
    };
    SDL_RenderTexture(renderer_, texture.get(), &src, &dst);
}

void Renderer2D::render_world_layer(const Texture& texture, const Camera& camera)
{
    const float view_width = static_cast<float>(kLogicalWidth) / world_zoom(camera);
    const float max_source_x = std::max(0.0f, texture.width() - view_width);
    const float source_x = clamp_float(camera.render_x(), 0.0f, max_source_x);
    const SDL_FRect src{source_x, 0.0f, std::min(view_width, texture.width() - source_x), texture.height()};
    const SDL_FRect dst{
        0.0f,
        world_origin_y(camera) + camera.render_y(),
        static_cast<float>(kLogicalWidth),
        scale_world_length(static_cast<float>(kLogicalHeight), camera)
    };
    SDL_RenderTexture(renderer_, texture.get(), &src, &dst);
}

void Renderer2D::render_blood_particle(float x, float y, const Camera& camera)
{
    SDL_SetRenderDrawColor(renderer_, 188, 22, 24, 255);
    SDL_RenderPoint(renderer_, std::round(world_to_screen_x(x, camera)), std::round(world_to_screen_y(y, camera)));
}

void Renderer2D::render_smoke_particle(
    const Texture& texture,
    float x,
    float y,
    float size,
    float alpha,
    const Camera& camera)
{
    SDL_FRect dst{
        std::round(world_to_screen_x(x, camera) - scale_world_length(size, camera) * 0.5f),
        std::round(world_to_screen_y(y, camera) - scale_world_length(size, camera) * 0.5f),
        scale_world_length(size, camera),
        scale_world_length(size, camera)
    };
    SDL_SetTextureAlphaMod(texture.get(), static_cast<Uint8>(std::round(255.0f * clamp_float(alpha, 0.0f, 1.0f))));
    SDL_RenderTexture(renderer_, texture.get(), nullptr, &dst);
    SDL_SetTextureAlphaMod(texture.get(), 255);
}

void Renderer2D::render_grenade(const Grenade& grenade, const Camera& camera)
{
    if (!grenade.active) {
        return;
    }

    const float dx = grenade.x - grenade.prev_x;
    const float dy = grenade.y - grenade.prev_y;
    const float length = scale_world_length(kGrenadeRenderLength, camera);
    const float speed = std::sqrt(dx * dx + dy * dy);
    const float start_x = std::round(world_to_screen_x(grenade.x, camera));
    const float start_y = std::round(world_to_screen_y(grenade.y, camera));
    if (speed <= 0.0f) {
        SDL_SetRenderDrawColor(renderer_, 255, 226, 80, 255);
        SDL_RenderPoint(renderer_, start_x, start_y);
        return;
    }

    float end_x = start_x - dx / speed * length;
    float end_y = start_y - dy / speed * length;
    if (grenade.clip_tail) {
        const float clip_x = std::round(world_to_screen_x(grenade.clip_x, camera));
        const float clip_y = std::round(world_to_screen_y(grenade.clip_y, camera));
        const float clip_distance = std::sqrt((clip_x - start_x) * (clip_x - start_x) + (clip_y - start_y) * (clip_y - start_y));
        if (clip_distance < length) {
            end_x = clip_x;
            end_y = clip_y;
        }
    }
    SDL_SetRenderDrawColor(renderer_, 255, 226, 80, 255);
    SDL_RenderLine(renderer_, start_x, start_y, end_x, end_y);
}

void Renderer2D::render_explosion(const Assets& assets, const Explosion& explosion, const Camera& camera)
{
    if (!explosion.active) {
        return;
    }

    const int frame_index = static_cast<int>(explosion.frame);
    if (frame_index < 0 || frame_index >= kGrenadeExplosionFrameCount) {
        return;
    }

    const bool use_sheet = assets.has_explosion_sheet &&
        frame_index < assets.explosion_sheet_meta.frame_count();
    SDL_FRect dst{};
    double angle = 0.0;
    SDL_FlipMode flip = SDL_FLIP_NONE;

    switch (explosion.direction) {
    case SurfaceImpactDirection::Top:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 49.0f, camera), world_to_screen_y(explosion.y - 93.0f, camera), scale_world_length(120.0f, camera), scale_world_length(109.0f, camera)};
        break;
    case SurfaceImpactDirection::Bottom:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 49.0f, camera), world_to_screen_y(explosion.y - 16.0f, camera), scale_world_length(120.0f, camera), scale_world_length(109.0f, camera)};
        flip = SDL_FLIP_VERTICAL;
        break;
    case SurfaceImpactDirection::Left:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 93.0f, camera), world_to_screen_y(explosion.y - 71.0f, camera), scale_world_length(120.0f, camera), scale_world_length(109.0f, camera)};
        angle = -90.0;
        break;
    case SurfaceImpactDirection::Right:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 16.0f, camera), world_to_screen_y(explosion.y - 49.0f, camera), scale_world_length(120.0f, camera), scale_world_length(109.0f, camera)};
        angle = 90.0;
        break;
    case SurfaceImpactDirection::None:
    default:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 49.0f, camera), world_to_screen_y(explosion.y - 93.0f, camera), scale_world_length(120.0f, camera), scale_world_length(109.0f, camera)};
        break;
    }

    const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
    if (use_sheet) {
        const SDL_FRect src = assets.explosion_sheet_meta.frame_rect(frame_index);
        SDL_RenderTextureRotated(renderer_, assets.explosion_sheet.get(), &src, &dst, angle, &center, flip);
    } else {
        SDL_RenderTextureRotated(renderer_, assets.explosions[frame_index].get(), nullptr, &dst, angle, &center, flip);
    }
}

void Renderer2D::render_player(const Texture& hero, const Player& player, float alpha, const Camera& camera)
{
    SDL_FRect src{sprites::hero_walk_sheet().frame_rect(player.walk_frame)};
    SDL_FRect dst{std::round(world_to_screen_x(player.x, camera)), world_to_screen_y(player.y, camera), scale_world_length(18.0f, camera), scale_world_length(33.0f, camera)};
    const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_SetTextureAlphaMod(hero.get(), static_cast<Uint8>(std::round(clamp_float(alpha, 0.0f, 1.0f) * 255.0f)));
    SDL_RenderTextureRotated(renderer_, hero.get(), &src, &dst, 0.0, &center, flip);
    SDL_SetTextureAlphaMod(hero.get(), 255);
}

void Renderer2D::render_weapon(const Texture& weapon, const WeaponDefinition& definition, const Player& player, float alpha, const Camera& camera)
{
    const float frame_width = weapon.width() * 0.5f;
    const float frame_height = weapon.height();
    const SDL_FRect src{0.0f, 0.0f, frame_width, frame_height};

    const float pivot_world_x = player.x + (player.facing_right ? 8.0f : 10.0f);
    const float pivot_world_y = player.y + 13.0f;
    const float pivot_screen_x = std::round(world_to_screen_x(pivot_world_x, camera));
    const float pivot_screen_y = std::round(world_to_screen_y(pivot_world_y, camera));
    const SDL_FPoint pivot{
        scale_world_length(player.facing_right ? static_cast<float>(definition.route_x) : frame_width - static_cast<float>(definition.route_x), camera),
        scale_world_length(static_cast<float>(definition.route_y), camera)
    };
    const SDL_FRect dst{
        pivot_screen_x - pivot.x,
        pivot_screen_y - pivot.y,
        scale_world_length(frame_width, camera),
        scale_world_length(frame_height, camera)
    };
    const double aim_degrees = static_cast<double>(player.aim_angle_radians * 180.0f / 3.1415926535f);
    const double angle_degrees = player.facing_right ? -aim_degrees : 180.0 - aim_degrees;
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_SetTextureAlphaMod(weapon.get(), static_cast<Uint8>(std::round(clamp_float(alpha, 0.0f, 1.0f) * 255.0f)));
    SDL_RenderTextureRotated(renderer_, weapon.get(), &src, &dst, angle_degrees, &pivot, flip);
    SDL_SetTextureAlphaMod(weapon.get(), 255);
}

void Renderer2D::render_bullet(const Bullet& bullet, const Camera& camera)
{
    if (!bullet.active) {
        return;
    }

    const float start_x = std::round(world_to_screen_x(bullet.x, camera));
    const float start_y = std::round(world_to_screen_y(bullet.y, camera));
    const float length = scale_world_length(18.0f, camera);
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
        SDL_FRect src{sprites::zombie_walk_sheet().frame_rect(zombie.walk_frame)};
        SDL_FRect dst{std::round(world_to_screen_x(zombie.x, camera)), world_to_screen_y(zombie.y, camera), scale_world_length(kZombieWidth, camera), scale_world_length(kZombieHeight, camera)};
        const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
        const SDL_FlipMode flip = zombie.walking_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, 0.0, &center, flip);
    } else {
        const SDL_FRect src{sprites::zombie_corpse_frame()};
        const SDL_FRect dst{
            std::round(world_to_screen_x(zombie.x + 7.5f, camera)),
            world_to_screen_y(zombie.y + kZombieCorpseRenderYOffset, camera),
            scale_world_length(17.0f, camera),
            scale_world_length(32.0f, camera)
        };
        const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
        const SDL_FlipMode flip = zombie.walking_right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_SetTextureAlphaMod(texture.get(), static_cast<Uint8>(std::round(zombie.corpse_alpha)));
        SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, zombie.corpse_angle_degrees, &center, flip);
        SDL_SetTextureAlphaMod(texture.get(), 255);
    }

    if (zombie.hit_flash > 0.0f) {
        SDL_SetTextureColorMod(texture.get(), 255, 255, 255);
    }
}

} // namespace zg
