#include "HudRenderer.h"

#include "Camera.h"
#include "Constants.h"
#include "MathUtil.h"
#include "Player.h"
#include "Texture.h"
#include "Weapon.h"

#include <cmath>
#include <cstdio>

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

HudRenderer::HudRenderer(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    hud_font_ = load_hud_font();
}

HudRenderer::~HudRenderer()
{
    if (hud_font_ != nullptr) {
        TTF_CloseFont(hud_font_);
        hud_font_ = nullptr;
    }
}

void HudRenderer::render_weapon_status(
    const Texture& bullet_icon,
    const Player& player,
    const WeaponDefinition* weapon_definition,
    const WeaponState& weapon,
    const Camera& camera)
{
    const float center_x = std::round(player.x - camera.render_x() + 10.0f);
    const float center_y = std::round(player.y - 10.0f + camera.render_y());
    const float ratio = weapon.indicator_ratio();

    if (weapon.reloading && weapon.reload_flash_on) {
        SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    }
    if (ratio > 0.0f) {
        draw_pie(renderer_, center_x, center_y, kHudIndicatorRadius, ratio);
    }

    const SDL_FRect icon_dst{
        std::round(player.x - camera.render_x() + 24.0f),
        std::round(player.y - 20.0f + camera.render_y()),
        7.0f,
        11.0f
    };
    SDL_RenderTexture(renderer_, bullet_icon.get(), nullptr, &icon_dst);

    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%d/%d", weapon.ammo_in_mag, weapon.ammo_reserve);
    const float text_x = std::round(player.x - camera.render_x() + 34.0f);
    const float text_y = std::round(player.y - 19.0f + camera.render_y());
    if (weapon_definition != nullptr) {
        render_text(weapon_definition->name.c_str(), text_x, text_y - 12.0f);
    }
    render_text(buffer, text_x, text_y - 1.0f);
}

void HudRenderer::render_top_bar(const WeaponState& weapon, int wave, int alive_count)
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    const SDL_FRect panel{8.0f, 8.0f, 172.0f, 42.0f};
    SDL_SetRenderDrawColor(renderer_, 8, 12, 18, 180);
    SDL_RenderFillRect(renderer_, &panel);
    SDL_SetRenderDrawColor(renderer_, 78, 84, 96, 220);
    SDL_RenderRect(renderer_, &panel);

    char wave_text[32];
    std::snprintf(wave_text, sizeof(wave_text), "Wave %d", wave);
    char alive_text[32];
    std::snprintf(alive_text, sizeof(alive_text), "Alive %d", alive_count);
    render_text(wave_text, 16.0f, 14.0f);
    render_text(alive_text, 16.0f, 27.0f);
    render_slot_bar(weapon, 86.0f, 15.0f);

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void HudRenderer::render_title_screen(float ui_alpha, bool show_prompt)
{
    const Uint8 alpha = static_cast<Uint8>(std::round(clamp_float(ui_alpha, 0.0f, 1.0f) * 255.0f));
    if (alpha == 0) {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    const SDL_FRect panel{408.0f, 84.0f, 196.0f, 132.0f};
    SDL_SetRenderDrawColor(renderer_, 8, 12, 18, static_cast<Uint8>(alpha * 0.78f));
    SDL_RenderFillRect(renderer_, &panel);
    SDL_SetRenderDrawColor(renderer_, 102, 112, 126, alpha);
    SDL_RenderRect(renderer_, &panel);

    render_text("Zombie Game", 428.0f, 102.0f);
    render_text("Survive the night", 428.0f, 122.0f);
    render_text("Defend the building", 428.0f, 138.0f);
    if (show_prompt) {
        render_text("Click or Press Enter", 428.0f, 172.0f);
    }

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void HudRenderer::render_text(const char* text, float x, float y)
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

void HudRenderer::render_slot_bar(const WeaponState& weapon, float x, float y)
{
    const float slot_width = 16.0f;
    const float slot_height = 12.0f;
    const float gap = 4.0f;

    for (int i = 0; i < weapon.slot_count(); ++i) {
        const float slot_x = x + i * (slot_width + gap);
        const SDL_FRect rect{slot_x, y, slot_width, slot_height};
        const bool active = i == weapon.active_slot_index();
        if (active) {
            SDL_SetRenderDrawColor(renderer_, 236, 216, 88, 255);
            SDL_RenderFillRect(renderer_, &rect);
            SDL_SetRenderDrawColor(renderer_, 32, 24, 8, 255);
        } else {
            SDL_SetRenderDrawColor(renderer_, 20, 26, 34, 220);
            SDL_RenderFillRect(renderer_, &rect);
            SDL_SetRenderDrawColor(renderer_, 136, 142, 150, 255);
            SDL_RenderRect(renderer_, &rect);
            SDL_SetRenderDrawColor(renderer_, 220, 224, 232, 255);
        }

        char label[4];
        std::snprintf(label, sizeof(label), "%d", i + 1);
        render_text(label, slot_x + 5.0f, y + 1.0f);
    }
}

} // namespace zg
