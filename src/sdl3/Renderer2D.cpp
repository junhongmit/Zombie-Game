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
#include <array>
#include <cstdio>
#include <cmath>

namespace zg {

namespace {

void draw_pie(SDL_Renderer* renderer, float center_x, float center_y, float radius, float ratio)
{
    const float start_angle = 90.0f * 3.1415926535f / 180.0f;
    const float clamped_ratio = clamp_float(ratio, 0.0f, 1.0f);
    const float end_angle = start_angle - clamped_ratio * 2.0f * 3.1415926535f;
    for (float angle = start_angle; angle >= end_angle; angle -= 0.04f) {
        const float px = center_x + std::cos(angle) * radius;
        const float py = center_y - std::sin(angle) * radius;
        SDL_RenderLine(renderer, center_x, center_y, px, py);
    }
}

void draw_digit(SDL_Renderer* renderer, int digit, float x, float y, int scale)
{
    static const std::array<const char*, 10> kDigits = {{
        "111"
        "101"
        "101"
        "101"
        "111",
        "010"
        "110"
        "010"
        "010"
        "111",
        "111"
        "001"
        "111"
        "100"
        "111",
        "111"
        "001"
        "111"
        "001"
        "111",
        "101"
        "101"
        "111"
        "001"
        "001",
        "111"
        "100"
        "111"
        "001"
        "111",
        "111"
        "100"
        "111"
        "101"
        "111",
        "111"
        "001"
        "001"
        "001"
        "001",
        "111"
        "101"
        "111"
        "101"
        "111",
        "111"
        "101"
        "111"
        "001"
        "111",
    }};

    if (digit < 0 || digit > 9) {
        return;
    }

    const char* pattern = kDigits[digit];
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (pattern[row * 3 + col] != '1') {
                continue;
            }
            const SDL_FRect rect{
                x + static_cast<float>(col * scale),
                y + static_cast<float>(row * scale),
                static_cast<float>(scale),
                static_cast<float>(scale)
            };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void draw_number(SDL_Renderer* renderer, int value, float x, float y, int scale)
{
    const char hundreds = static_cast<char>('0' + ((value / 100) % 10));
    const char tens = static_cast<char>('0' + ((value / 10) % 10));
    const char ones = static_cast<char>('0' + (value % 10));

    float cursor_x = x;
    if (value >= 100) {
        draw_digit(renderer, hundreds - '0', cursor_x, y, scale);
        cursor_x += 4.0f * scale;
    }
    if (value >= 10) {
        draw_digit(renderer, tens - '0', cursor_x, y, scale);
        cursor_x += 4.0f * scale;
    }
    draw_digit(renderer, ones - '0', cursor_x, y, scale);
}

void draw_slash(SDL_Renderer* renderer, float x, float y, int scale)
{
    SDL_RenderLine(
        renderer,
        x,
        y + 4.0f * scale,
        x + 2.0f * scale,
        y);
}

TTF_Font* load_hud_font()
{
    static const char* kCandidates[] = {
        "C:\\Windows\\Fonts\\consola.ttf",
        "C:\\Windows\\Fonts\\lucon.ttf",
        "C:\\Windows\\Fonts\\cour.ttf",
        "C:\\Windows\\Fonts\\msyh.ttc",
    };

    for (const char* path : kCandidates) {
        TTF_Font* font = TTF_OpenFont(path, kHudFontPointSize);
        if (font != nullptr) {
            return font;
        }
    }
    return nullptr;
}

} // namespace

Renderer2D::Renderer2D(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    hud_font_ = load_hud_font();
}

Renderer2D::~Renderer2D()
{
    if (hud_font_ != nullptr) {
        TTF_CloseFont(hud_font_);
        hud_font_ = nullptr;
    }
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
    const WeaponState& weapon,
    const Camera& camera)
{
    render_fullscreen(assets.sky);
    render_scrolling_layer(assets.backcity3, camera.render_x(), 0.2f, 316.0f + camera.render_y(), 116.0f);
    render_scrolling_layer(assets.backcity2, camera.render_x(), 0.25f, 316.0f + camera.render_y(), 116.0f);
    render_scrolling_layer(assets.backcity1, camera.render_x(), 0.5f, 316.0f + camera.render_y(), 116.0f);
    render_world_layer(assets.building, camera.render_x());

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
        render_explosion(assets.explosions, explosions[i], camera);
    }
    render_player(assets.hero, player, camera);
    if (weapon_definition != nullptr) {
        render_weapon(weapon_definition->texture, *weapon_definition, player, camera);
    }
    render_weapon_hud(assets.bullet_icon, player, weapon_definition, weapon, camera);

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

void Renderer2D::render_weapon_hud(
    const Texture& bullet_icon,
    const Player& player,
    const WeaponDefinition* weapon_definition,
    const WeaponState& weapon,
    const Camera& camera)
{
    const float center_x = std::round(player.x - camera.render_x() + 10.0f);
    const float center_y = std::round(player.y - 10.0f + camera.render_y());
    const float radius = kHudIndicatorRadius;
    const float ratio = weapon.indicator_ratio();

    if (weapon.reloading && weapon.reload_flash_on) {
        SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    }
    if (ratio > 0.0f) {
        draw_pie(renderer_, center_x, center_y, radius, ratio);
    }

    const SDL_FRect icon_dst{std::round(player.x - camera.render_x() + 24.0f), std::round(player.y - 20.0f + camera.render_y()), 7.0f, 11.0f};
    SDL_RenderTexture(renderer_, bullet_icon.get(), nullptr, &icon_dst);
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    const float text_x = std::round(player.x - camera.render_x() + 34.0f);
    const float text_y = std::round(player.y - 19.0f + camera.render_y());
    if (hud_font_ != nullptr) {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%d/%d", weapon.ammo_in_mag, weapon.ammo_reserve);
        if (weapon_definition != nullptr) {
            render_hud_text(weapon_definition->name.c_str(), text_x, text_y - 12.0f);
        }
        render_hud_text(buffer, text_x, text_y - 1.0f);
    } else {
        draw_number(renderer_, weapon.ammo_in_mag, text_x, text_y, kHudDigitScale);
        draw_slash(renderer_, text_x + 14.0f, text_y, kHudDigitScale);
        draw_number(renderer_, weapon.ammo_reserve, text_x + 20.0f, text_y, kHudDigitScale);
    }
}

void Renderer2D::render_hud_text(const char* text, float x, float y)
{
    if (hud_font_ == nullptr || text == nullptr || text[0] == '\0') {
        return;
    }

    const SDL_Color color{255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(hud_font_, text, 0, color);
    if (surface == nullptr) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (texture == nullptr) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    const SDL_FRect dst{
        x,
        y,
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };
    SDL_RenderTexture(renderer_, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
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

void Renderer2D::render_blood_particle(float x, float y, const Camera& camera)
{
    SDL_SetRenderDrawColor(renderer_, 188, 22, 24, 255);
    SDL_RenderPoint(renderer_, std::round(x - camera.render_x()), std::round(y + camera.render_y()));
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
        std::round(x - camera.render_x() - size * 0.5f),
        std::round(y - size * 0.5f + camera.render_y()),
        size,
        size
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
    const float length = kGrenadeRenderLength;
    const float speed = std::sqrt(dx * dx + dy * dy);
    const float start_x = std::round(grenade.x - camera.render_x());
    const float start_y = std::round(grenade.y + camera.render_y());
    if (speed <= 0.0f) {
        SDL_SetRenderDrawColor(renderer_, 255, 226, 80, 255);
        SDL_RenderPoint(renderer_, start_x, start_y);
        return;
    }

    float end_x = start_x - dx / speed * length;
    float end_y = start_y - dy / speed * length;
    if (grenade.clip_tail) {
        const float clip_x = std::round(grenade.clip_x - camera.render_x());
        const float clip_y = std::round(grenade.clip_y + camera.render_y());
        const float clip_distance = std::sqrt((clip_x - start_x) * (clip_x - start_x) + (clip_y - start_y) * (clip_y - start_y));
        if (clip_distance < length) {
            end_x = clip_x;
            end_y = clip_y;
        }
    }
    SDL_SetRenderDrawColor(renderer_, 255, 226, 80, 255);
    SDL_RenderLine(renderer_, start_x, start_y, end_x, end_y);
}

void Renderer2D::render_explosion(const Texture* frames, const Explosion& explosion, const Camera& camera)
{
    if (!explosion.active) {
        return;
    }

    const int frame_index = static_cast<int>(explosion.frame);
    if (frame_index < 0 || frame_index >= kGrenadeExplosionFrameCount) {
        return;
    }

    const Texture& frame = frames[frame_index];
    SDL_FRect dst{};
    double angle = 0.0;
    SDL_FlipMode flip = SDL_FLIP_NONE;

    switch (explosion.direction) {
    case SurfaceImpactDirection::Top:
        dst = SDL_FRect{explosion.x - 49.0f - camera.render_x(), explosion.y - 93.0f + camera.render_y(), 120.0f, 109.0f};
        break;
    case SurfaceImpactDirection::Bottom:
        dst = SDL_FRect{explosion.x - 49.0f - camera.render_x(), explosion.y - 16.0f + camera.render_y(), 120.0f, 109.0f};
        flip = SDL_FLIP_VERTICAL;
        break;
    case SurfaceImpactDirection::Left:
        dst = SDL_FRect{explosion.x - 93.0f - camera.render_x(), explosion.y - 71.0f + camera.render_y(), 120.0f, 109.0f};
        angle = -90.0;
        break;
    case SurfaceImpactDirection::Right:
        dst = SDL_FRect{explosion.x - 16.0f - camera.render_x(), explosion.y - 49.0f + camera.render_y(), 120.0f, 109.0f};
        angle = 90.0;
        break;
    case SurfaceImpactDirection::None:
    default:
        dst = SDL_FRect{explosion.x - 49.0f - camera.render_x(), explosion.y - 93.0f + camera.render_y(), 120.0f, 109.0f};
        break;
    }

    const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
    SDL_RenderTextureRotated(renderer_, frame.get(), nullptr, &dst, angle, &center, flip);
}

void Renderer2D::render_player(const Texture& hero, const Player& player, const Camera& camera)
{
    SDL_FRect src{sprites::hero_walk_sheet().frame_rect(player.walk_frame)};
    SDL_FRect dst{std::round(player.x - camera.render_x()), player.y + camera.render_y(), 18.0f, 33.0f};
    const SDL_FPoint center{9.0f, 16.5f};
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderTextureRotated(renderer_, hero.get(), &src, &dst, 0.0, &center, flip);
}

void Renderer2D::render_weapon(const Texture& weapon, const WeaponDefinition& definition, const Player& player, const Camera& camera)
{
    const float frame_width = weapon.width() * 0.5f;
    const float frame_height = weapon.height();
    const SDL_FRect src{0.0f, 0.0f, frame_width, frame_height};

    const float pivot_screen_x = std::round(player.x - camera.render_x() + (player.facing_right ? 8.0f : 10.0f));
    const float pivot_screen_y = std::round(player.y + 13.0f + camera.render_y());
    const SDL_FPoint pivot{
        player.facing_right ? static_cast<float>(definition.route_x) : frame_width - static_cast<float>(definition.route_x),
        static_cast<float>(definition.route_y)
    };
    const SDL_FRect dst{pivot_screen_x - pivot.x, pivot_screen_y - pivot.y, frame_width, frame_height};
    const double aim_degrees = static_cast<double>(player.aim_angle_radians * 180.0f / 3.1415926535f);
    const double angle_degrees = player.facing_right ? -aim_degrees : 180.0 - aim_degrees;
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderTextureRotated(renderer_, weapon.get(), &src, &dst, angle_degrees, &pivot, flip);
}

void Renderer2D::render_bullet(const Bullet& bullet, const Camera& camera)
{
    if (!bullet.active) {
        return;
    }

    const float start_x = std::round(bullet.x - camera.render_x());
    const float start_y = std::round(bullet.y + camera.render_y());
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
        SDL_FRect src{sprites::zombie_walk_sheet().frame_rect(zombie.walk_frame)};
        SDL_FRect dst{std::round(zombie.x - camera.render_x()), zombie.y + camera.render_y(), kZombieWidth, kZombieHeight};
        const SDL_FPoint center{9.0f, 16.0f};
        const SDL_FlipMode flip = zombie.walking_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, 0.0, &center, flip);
    } else {
        const SDL_FRect src{sprites::zombie_corpse_frame()};
        const SDL_FRect dst{
            std::round(zombie.x - camera.render_x() + 7.5f),
            zombie.y + kZombieCorpseRenderYOffset + camera.render_y(),
            17.0f,
            32.0f
        };
        const SDL_FPoint center{8.5f, 16.0f};
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
