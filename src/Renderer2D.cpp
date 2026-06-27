#include "Renderer2D.h"

#include "Assets.h"
#include "Bullet.h"
#include "Camera.h"
#include "Constants.h"
#include "Effects.h"
#include "Grenade.h"
#include "MathUtil.h"
#include "Player.h"
#include "Presentation.h"
#include "SpriteCatalog.h"
#include "Texture.h"
#include "UpperBodyAimingRig.h"
#include "Weapon.h"
#include "Zombie.h"

#include <algorithm>
#include <cmath>

namespace zg {

namespace {

float ui_scale(const SDL_FRect& presentation_rect)
{
    return presentation_scale(presentation_rect);
}

float world_zoom(const Camera& camera, const SDL_FRect& presentation_rect)
{
    return camera.render_zoom() * ui_scale(presentation_rect);
}

float world_origin_y(const Camera& camera, const SDL_FRect& presentation_rect)
{
    return presentation_rect.y + presentation_rect.h -
        kGameplayViewHeight * world_zoom(camera, presentation_rect);
}

float world_to_screen_x(float world_x, const Camera& camera, const SDL_FRect& presentation_rect)
{
    return presentation_rect.x + (world_x - camera.render_x()) * world_zoom(camera, presentation_rect);
}

float world_to_screen_y(float world_y, const Camera& camera, const SDL_FRect& presentation_rect)
{
    return world_y * world_zoom(camera, presentation_rect) +
        world_origin_y(camera, presentation_rect) +
        camera.render_y() * ui_scale(presentation_rect);
}

float scale_world_length(float value, const Camera& camera, const SDL_FRect& presentation_rect)
{
    return value * world_zoom(camera, presentation_rect);
}

SDL_FPoint resolve_rig_anchor(const UpperBodyAimingRigState& rig, const std::string& anchor)
{
    if (anchor == "torso_top") {
        return rig.torso_top;
    }
    if (anchor == "torso_base") {
        return rig.torso_base;
    }
    if (anchor == "pelvis") {
        return rig.pelvis;
    }
    if (anchor == "head_center") {
        return rig.head_center;
    }
    if (anchor == "front_shoulder") {
        return rig.front_shoulder;
    }
    if (anchor == "back_shoulder") {
        return rig.back_shoulder;
    }
    if (anchor == "front_elbow") {
        return rig.front_arm.elbow;
    }
    if (anchor == "back_elbow") {
        return rig.back_arm.elbow;
    }
    if (anchor == "rear_grip") {
        return rig.rear_grip;
    }
    if (anchor == "front_grip") {
        return rig.front_grip;
    }
    if (anchor == "muzzle") {
        return rig.muzzle;
    }
    return rig.torso_top;
}

void render_texture_mirrored_to_rect(SDL_Renderer* renderer, const Texture& texture, const SDL_FRect& dst_rect)
{
    if (!texture.valid() || dst_rect.w <= 0.0f || dst_rect.h <= 0.0f) {
        return;
    }

    const float texture_aspect = texture.width() / std::max(1.0f, texture.height());
    const float target_aspect = dst_rect.w / std::max(1.0f, dst_rect.h);

    if (std::fabs(texture_aspect - target_aspect) < 0.001f) {
        SDL_RenderTexture(renderer, texture.get(), nullptr, &dst_rect);
        return;
    }

    if (texture_aspect < target_aspect) {
        const float center_width = dst_rect.h * texture_aspect;
        const float side_width = std::max(0.0f, (dst_rect.w - center_width) * 0.5f);
        const SDL_FRect center_dst{
            dst_rect.x + std::floor((dst_rect.w - center_width) * 0.5f),
            dst_rect.y,
            center_width,
            dst_rect.h
        };
        SDL_RenderTexture(renderer, texture.get(), nullptr, &center_dst);

        if (side_width > 0.0f) {
            const float strip_width = std::max(1.0f, std::floor(texture.width() * 0.125f));
            const SDL_FRect left_src{0.0f, 0.0f, strip_width, texture.height()};
            const SDL_FRect right_src{texture.width() - strip_width, 0.0f, strip_width, texture.height()};
            const SDL_FRect left_dst{dst_rect.x, dst_rect.y, side_width, dst_rect.h};
            const SDL_FRect right_dst{dst_rect.x + dst_rect.w - side_width, dst_rect.y, side_width, dst_rect.h};
            const SDL_FPoint center{0.0f, 0.0f};
            SDL_RenderTextureRotated(renderer, texture.get(), &left_src, &left_dst, 0.0, &center, SDL_FLIP_HORIZONTAL);
            SDL_RenderTextureRotated(renderer, texture.get(), &right_src, &right_dst, 0.0, &center, SDL_FLIP_HORIZONTAL);
        }
        return;
    }

    const float center_height = dst_rect.w / texture_aspect;
    const float side_height = std::max(0.0f, (dst_rect.h - center_height) * 0.5f);
    const SDL_FRect center_dst{
        dst_rect.x,
        dst_rect.y + std::floor((dst_rect.h - center_height) * 0.5f),
        dst_rect.w,
        center_height
    };
    SDL_RenderTexture(renderer, texture.get(), nullptr, &center_dst);

    if (side_height > 0.0f) {
        const float strip_height = std::max(1.0f, std::floor(texture.height() * 0.125f));
        const SDL_FRect top_src{0.0f, 0.0f, texture.width(), strip_height};
        const SDL_FRect bottom_src{0.0f, texture.height() - strip_height, texture.width(), strip_height};
        const SDL_FRect top_dst{dst_rect.x, dst_rect.y, dst_rect.w, side_height};
        const SDL_FRect bottom_dst{dst_rect.x, dst_rect.y + dst_rect.h - side_height, dst_rect.w, side_height};
        const SDL_FPoint center{0.0f, 0.0f};
        SDL_RenderTextureRotated(renderer, texture.get(), &top_src, &top_dst, 0.0, &center, SDL_FLIP_VERTICAL);
        SDL_RenderTextureRotated(renderer, texture.get(), &bottom_src, &bottom_dst, 0.0, &center, SDL_FLIP_VERTICAL);
    }
}

} // namespace

Renderer2D::Renderer2D(SDL_Renderer* renderer)
    : renderer_(renderer)
{
}

Renderer2D::~Renderer2D()
{
    if (frame_target_ != nullptr) {
        SDL_DestroyTexture(frame_target_);
        frame_target_ = nullptr;
    }
}

bool Renderer2D::ensure_render_target()
{
    if (frame_target_ != nullptr) {
        return true;
    }

    frame_target_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        kInternalRenderWidth,
        kInternalRenderHeight);
    if (frame_target_ == nullptr) {
        return false;
    }

    SDL_SetTextureScaleMode(frame_target_, SDL_SCALEMODE_LINEAR);
    return true;
}

void Renderer2D::begin_frame()
{
    if (!ensure_render_target()) {
        presentation_rect_ = SDL_FRect{};
        return;
    }

    presentation_rect_ = compute_presentation_rect(kInternalRenderWidth, kInternalRenderHeight);

    SDL_SetRenderTarget(renderer_, frame_target_);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_SetRenderDrawColor(renderer_, 11, 18, 26, 255);
    SDL_RenderFillRect(renderer_, &presentation_rect_);
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
    const bool use_rig_player = show_player && assets.hero_rig.loaded && weapon_definition != nullptr;
    if (use_rig_player) {
        render_player_rig(assets, player, weapon_definition, player_alpha, camera);
    } else if (show_player) {
        render_player(assets.hero, player, player_alpha, camera);
        if (weapon_definition != nullptr) {
            render_weapon(weapon_definition->texture, *weapon_definition, player, player_alpha, camera);
        }
    }
    if (show_player && kEnableAimingRigDebug) {
        UpperBodyAimingRig rig_solver;
        render_aiming_rig_debug(rig_solver.solve(player, weapon_definition, &assets.hero_rig), camera);
    }

    for (int i = 0; i < effects.smoke_particle_count(); ++i) {
        const SmokeParticle& particle = effects.smoke_particles()[i];
        if (!particle.active) {
            continue;
        }
        const float t = particle.age / particle.lifetime;
        const float size = kSmokeBaseSize + kSmokeGrowthSize * t;
        const float alpha = 1.0f - t;
        render_smoke_particle(assets.smoke, particle.x, particle.y, size, alpha, camera);
    }
}

void Renderer2D::render_rig_preview(
    const Assets& assets,
    const Player& player,
    const WeaponDefinition* weapon_definition,
    const Camera& camera)
{
    SDL_SetRenderDrawColor(renderer_, 17, 18, 22, 255);
    SDL_RenderFillRect(renderer_, &presentation_rect_);

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 86, 70, 48, 48);
    const float grid = scale_world_length(8.0f, camera, presentation_rect_);
    if (grid > 1.0f) {
        for (float x = std::fmod(presentation_rect_.x - world_to_screen_x(0.0f, camera, presentation_rect_), grid);
             x < presentation_rect_.w;
             x += grid) {
            SDL_RenderLine(renderer_, presentation_rect_.x + x, presentation_rect_.y, presentation_rect_.x + x, presentation_rect_.y + presentation_rect_.h);
        }
        for (float y = std::fmod(presentation_rect_.y - world_to_screen_y(0.0f, camera, presentation_rect_), grid);
             y < presentation_rect_.h;
             y += grid) {
            SDL_RenderLine(renderer_, presentation_rect_.x, presentation_rect_.y + y, presentation_rect_.x + presentation_rect_.w, presentation_rect_.y + y);
        }
    }

    SDL_SetRenderDrawColor(renderer_, 176, 126, 82, 120);
    const float floor_y = world_to_screen_y(player.y + 33.0f, camera, presentation_rect_);
    SDL_RenderLine(renderer_, presentation_rect_.x, floor_y, presentation_rect_.x + presentation_rect_.w, floor_y);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);

    render_player_rig(assets, player, weapon_definition, 1.0f, camera, true);
    if (assets.hero_rig.loaded) {
        UpperBodyAimingRig rig_solver;
        render_aiming_rig_debug(rig_solver.solve(player, weapon_definition, &assets.hero_rig), camera);
    }
}

void Renderer2D::end_frame()
{
    if (frame_target_ == nullptr) {
        return;
    }

    int output_width = 0;
    int output_height = 0;
    SDL_GetRenderOutputSize(renderer_, &output_width, &output_height);
    const SDL_FRect output_rect = compute_aspect_rect(
        output_width,
        output_height,
        kInternalRenderWidth,
        kInternalRenderHeight);

    SDL_SetRenderTarget(renderer_, nullptr);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, frame_target_, nullptr, &output_rect);
    SDL_RenderPresent(renderer_);
}

SDL_FRect Renderer2D::presentation_rect() const
{
    return presentation_rect_;
}

void Renderer2D::render_fullscreen(const Texture& texture)
{
    render_texture_mirrored_to_rect(renderer_, texture, presentation_rect_);
}

void Renderer2D::render_scrolling_layer(
    const Texture& texture,
    const Camera& camera,
    float parallax,
    float y,
    float height)
{
    const float zoom = camera.render_zoom();
    const float view_width = kGameplayViewWidth / zoom;
    const float max_source_x = std::max(0.0f, texture.width() - view_width);
    const float source_x = clamp_float(camera.render_x() * parallax, 0.0f, max_source_x);
    const SDL_FRect src{source_x, 0.0f, std::min(view_width, texture.width() - source_x), texture.height()};
    const SDL_FRect dst{
        presentation_rect_.x,
        world_to_screen_y(y, camera, presentation_rect_),
        presentation_rect_.w,
        scale_world_length(height, camera, presentation_rect_)
    };
    SDL_RenderTexture(renderer_, texture.get(), &src, &dst);
}

void Renderer2D::render_world_layer(const Texture& texture, const Camera& camera)
{
    const float view_width = kGameplayViewWidth / camera.render_zoom();
    const float max_source_x = std::max(0.0f, texture.width() - view_width);
    const float source_x = clamp_float(camera.render_x(), 0.0f, max_source_x);
    const SDL_FRect src{source_x, 0.0f, std::min(view_width, texture.width() - source_x), texture.height()};
    const SDL_FRect dst{
        presentation_rect_.x,
        world_origin_y(camera, presentation_rect_) + camera.render_y() * ui_scale(presentation_rect_),
        presentation_rect_.w,
        scale_world_length(kGameplayViewHeight, camera, presentation_rect_)
    };
    SDL_RenderTexture(renderer_, texture.get(), &src, &dst);
}

void Renderer2D::render_blood_particle(float x, float y, const Camera& camera)
{
    SDL_SetRenderDrawColor(renderer_, 188, 22, 24, 255);
    const float size = std::max(
        kBloodMinScreenSize,
        scale_world_length(kBloodRenderSize, camera, presentation_rect_));
    const SDL_FRect rect{
        std::round(world_to_screen_x(x, camera, presentation_rect_) - size * 0.5f),
        std::round(world_to_screen_y(y, camera, presentation_rect_) - size * 0.5f),
        size,
        size
    };
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer2D::render_smoke_particle(
    const Texture& texture,
    float x,
    float y,
    float size,
    float alpha,
    const Camera& camera)
{
    const float scaled_size = std::max(
        kSmokeMinScreenSize,
        scale_world_length(size, camera, presentation_rect_));
    SDL_FRect dst{
        std::round(world_to_screen_x(x, camera, presentation_rect_) - scaled_size * 0.5f),
        std::round(world_to_screen_y(y, camera, presentation_rect_) - scaled_size * 0.5f),
        scaled_size,
        scaled_size
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
    const float length = scale_world_length(kGrenadeRenderLength, camera, presentation_rect_);
    const float speed = std::sqrt(dx * dx + dy * dy);
    const float start_x = std::round(world_to_screen_x(grenade.x, camera, presentation_rect_));
    const float start_y = std::round(world_to_screen_y(grenade.y, camera, presentation_rect_));
    if (speed <= 0.0f) {
        SDL_SetRenderDrawColor(renderer_, 255, 226, 80, 255);
        SDL_RenderPoint(renderer_, start_x, start_y);
        return;
    }

    float end_x = start_x - dx / speed * length;
    float end_y = start_y - dy / speed * length;
    if (grenade.clip_tail) {
        const float clip_x = std::round(world_to_screen_x(grenade.clip_x, camera, presentation_rect_));
        const float clip_y = std::round(world_to_screen_y(grenade.clip_y, camera, presentation_rect_));
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
        dst = SDL_FRect{world_to_screen_x(explosion.x - 49.0f, camera, presentation_rect_), world_to_screen_y(explosion.y - 93.0f, camera, presentation_rect_), scale_world_length(120.0f, camera, presentation_rect_), scale_world_length(109.0f, camera, presentation_rect_)};
        break;
    case SurfaceImpactDirection::Bottom:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 49.0f, camera, presentation_rect_), world_to_screen_y(explosion.y - 16.0f, camera, presentation_rect_), scale_world_length(120.0f, camera, presentation_rect_), scale_world_length(109.0f, camera, presentation_rect_)};
        flip = SDL_FLIP_VERTICAL;
        break;
    case SurfaceImpactDirection::Left:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 93.0f, camera, presentation_rect_), world_to_screen_y(explosion.y - 71.0f, camera, presentation_rect_), scale_world_length(120.0f, camera, presentation_rect_), scale_world_length(109.0f, camera, presentation_rect_)};
        angle = -90.0;
        break;
    case SurfaceImpactDirection::Right:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 16.0f, camera, presentation_rect_), world_to_screen_y(explosion.y - 49.0f, camera, presentation_rect_), scale_world_length(120.0f, camera, presentation_rect_), scale_world_length(109.0f, camera, presentation_rect_)};
        angle = 90.0;
        break;
    case SurfaceImpactDirection::None:
    default:
        dst = SDL_FRect{world_to_screen_x(explosion.x - 49.0f, camera, presentation_rect_), world_to_screen_y(explosion.y - 93.0f, camera, presentation_rect_), scale_world_length(120.0f, camera, presentation_rect_), scale_world_length(109.0f, camera, presentation_rect_)};
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
    SDL_FRect dst{
        std::round(world_to_screen_x(player.x, camera, presentation_rect_)),
        world_to_screen_y(player.y, camera, presentation_rect_),
        scale_world_length(18.0f, camera, presentation_rect_),
        scale_world_length(33.0f, camera, presentation_rect_)
    };
    const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_SetTextureAlphaMod(hero.get(), static_cast<Uint8>(std::round(clamp_float(alpha, 0.0f, 1.0f) * 255.0f)));
    SDL_RenderTextureRotated(renderer_, hero.get(), &src, &dst, 0.0, &center, flip);
    SDL_SetTextureAlphaMod(hero.get(), 255);
}

void Renderer2D::render_weapon(const Texture& weapon, const WeaponDefinition& definition, const Player& player, float alpha, const Camera& camera)
{
    const float frame_width_pixels = definition.mirrored_pair_sprite
        ? weapon.width() * 0.5f
        : static_cast<float>(weapon.width());
    const float frame_height_pixels = static_cast<float>(weapon.height());
    const float frame_width = frame_width_pixels * definition.hold_scale;
    const float frame_height = frame_height_pixels * definition.hold_scale;
    const SDL_FRect src{0.0f, 0.0f, frame_width_pixels, frame_height_pixels};

    const float pivot_world_x = player.x + (player.facing_right ? 8.0f : 10.0f);
    const float pivot_world_y = player.y + 13.0f;
    const float pivot_screen_x = std::round(world_to_screen_x(pivot_world_x, camera, presentation_rect_));
    const float pivot_screen_y = std::round(world_to_screen_y(pivot_world_y, camera, presentation_rect_));
    const WeaponDefinition::LocalPoint wrist = player.facing_right
        ? definition.rear_wrist
        : WeaponDefinition::LocalPoint{frame_width_pixels - definition.rear_wrist.x, definition.rear_wrist.y};
    const SDL_FPoint pivot{
        scale_world_length(wrist.x * definition.hold_scale, camera, presentation_rect_),
        scale_world_length(wrist.y * definition.hold_scale, camera, presentation_rect_)
    };
    const SDL_FRect dst{
        pivot_screen_x - pivot.x,
        pivot_screen_y - pivot.y,
        scale_world_length(frame_width, camera, presentation_rect_),
        scale_world_length(frame_height, camera, presentation_rect_)
    };
    const double aim_degrees = static_cast<double>(player.aim_angle_radians * 180.0f / 3.1415926535f);
    const double angle_degrees = player.facing_right ? -aim_degrees : 180.0 - aim_degrees;
    const SDL_FlipMode flip = player.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_SetTextureAlphaMod(weapon.get(), static_cast<Uint8>(std::round(clamp_float(alpha, 0.0f, 1.0f) * 255.0f)));
    SDL_RenderTextureRotated(renderer_, weapon.get(), &src, &dst, angle_degrees, &pivot, flip);
    SDL_SetTextureAlphaMod(weapon.get(), 255);
}

void Renderer2D::render_weapon_with_rig(const Texture& weapon, const WeaponDefinition& definition, const UpperBodyAimingRigState& rig, float alpha, const Camera& camera)
{
    const float frame_width_pixels = definition.mirrored_pair_sprite
        ? weapon.width() * 0.5f
        : static_cast<float>(weapon.width());
    const float frame_height_pixels = static_cast<float>(weapon.height());
    const float frame_width = frame_width_pixels * definition.hold_scale;
    const float frame_height = frame_height_pixels * definition.hold_scale;
    const SDL_FRect src{0.0f, 0.0f, frame_width_pixels, frame_height_pixels};
    const float pivot_screen_x = std::round(world_to_screen_x(rig.rear_grip.x, camera, presentation_rect_));
    const float pivot_screen_y = std::round(world_to_screen_y(rig.rear_grip.y, camera, presentation_rect_));
    const WeaponDefinition::LocalPoint wrist = rig.facing_right
        ? definition.rear_wrist
        : WeaponDefinition::LocalPoint{frame_width_pixels - definition.rear_wrist.x, definition.rear_wrist.y};
    const SDL_FPoint pivot{
        scale_world_length(wrist.x * definition.hold_scale, camera, presentation_rect_),
        scale_world_length(wrist.y * definition.hold_scale, camera, presentation_rect_)
    };
    const SDL_FRect dst{
        pivot_screen_x - pivot.x,
        pivot_screen_y - pivot.y,
        scale_world_length(frame_width, camera, presentation_rect_),
        scale_world_length(frame_height, camera, presentation_rect_)
    };
    const double angle_degrees = rig.facing_right
        ? -static_cast<double>(rig.weapon_rotation_deg)
        : 180.0 - static_cast<double>(rig.weapon_rotation_deg);
    const SDL_FlipMode flip = rig.facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
    SDL_SetTextureAlphaMod(weapon.get(), static_cast<Uint8>(std::round(clamp_float(alpha, 0.0f, 1.0f) * 255.0f)));
    SDL_RenderTextureRotated(renderer_, weapon.get(), &src, &dst, angle_degrees, &pivot, flip);
    SDL_SetTextureAlphaMod(weapon.get(), 255);
}

void Renderer2D::render_player_rig(const Assets& assets, const Player& player, const WeaponDefinition* weapon_definition, float alpha, const Camera& camera, bool render_weapon_sprite)
{
    if (!assets.hero_rig.loaded || weapon_definition == nullptr) {
        render_player(assets.hero, player, alpha, camera);
        if (render_weapon_sprite && weapon_definition != nullptr) {
            render_weapon(weapon_definition->texture, *weapon_definition, player, alpha, camera);
        }
        return;
    }

    UpperBodyAimingRig rig_solver;
    const UpperBodyAimingRigState rig = rig_solver.solve(player, weapon_definition, &assets.hero_rig);
    if (!rig.valid) {
        render_player(assets.hero, player, alpha, camera);
        if (render_weapon_sprite) {
            render_weapon(weapon_definition->texture, *weapon_definition, player, alpha, camera);
        }
        return;
    }

    const auto render_part = [this, &camera, alpha, &rig](const Texture& sheet, const CharacterRigPart& part, SDL_FPoint world_anchor, float angle_deg, float scale_x, float scale_y) {
        const SDL_FRect src{
            static_cast<float>(part.frame.x),
            static_cast<float>(part.frame.y),
            static_cast<float>(part.frame.w),
            static_cast<float>(part.frame.h)
        };
        const float anchor_screen_x = std::round(world_to_screen_x(world_anchor.x, camera, presentation_rect_));
        const float anchor_screen_y = std::round(world_to_screen_y(world_anchor.y, camera, presentation_rect_));
        const bool flip_x = !rig.facing_right;
        const SDL_FPoint pivot{
            (flip_x ? (static_cast<float>(part.frame.w) - part.pivot.x) : part.pivot.x) * scale_x,
            part.pivot.y * scale_y
        };
        const SDL_FRect dst{
            anchor_screen_x - pivot.x,
            anchor_screen_y - pivot.y,
            part.frame.w * scale_x,
            part.frame.h * scale_y
        };
        SDL_SetTextureAlphaMod(sheet.get(), static_cast<Uint8>(std::round(clamp_float(alpha, 0.0f, 1.0f) * 255.0f)));
        SDL_RenderTextureRotated(renderer_, sheet.get(), &src, &dst, angle_deg, &pivot, flip_x ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(sheet.get(), 255);
    };

    const auto screen_segment_angle_deg = [](SDL_FPoint a, SDL_FPoint b) {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        return std::atan2(dy, dx) * 180.0f / 3.1415926535f;
    };
    const auto part_rotation_deg = [&screen_segment_angle_deg](const CharacterRigPart& part, SDL_FPoint a, SDL_FPoint b, bool flip_x) {
        const float world_angle = screen_segment_angle_deg(a, b);
        SDL_FPoint rest_dir{};
        if (part.has_distal_joint) {
            rest_dir = SDL_FPoint{
                (flip_x ? (part.pivot.x - part.distal_joint.x) : (part.distal_joint.x - part.pivot.x)),
                part.distal_joint.y - part.pivot.y
            };
        } else {
            rest_dir = SDL_FPoint{0.0f, 1.0f};
        }
        const float rest_angle = std::atan2(rest_dir.y, rest_dir.x) * 180.0f / 3.1415926535f;
        return world_angle - rest_angle;
    };

    const float torso_asset_length = std::max(1.0f, assets.hero_rig.torso_pelvis.y - assets.hero_rig.torso.pivot.y);
    const float torso_world_length = std::max(0.001f, rig.pelvis.y - rig.torso_top.y);
    const float rig_pixel_to_world = torso_world_length / torso_asset_length;
    const float rig_screen_scale = scale_world_length(rig_pixel_to_world, camera, presentation_rect_);

    const float torso_angle = screen_segment_angle_deg(rig.torso_top, rig.pelvis) - 90.0f;
    const float aim_t = std::min(1.0f, std::fabs(rig.aim_local_deg) / 75.0f);
    const float torso_scale_x = rig_screen_scale * (1.0f + ((rig.aim_local_deg >= 0.0f ? assets.hero_rig.torso_scale_x_max : assets.hero_rig.torso_scale_x_min) - 1.0f) * aim_t);
    const float torso_scale_y = rig_screen_scale * (1.0f + ((rig.aim_local_deg >= 0.0f ? assets.hero_rig.torso_scale_y_max : assets.hero_rig.torso_scale_y_min) - 1.0f) * aim_t);
    const float head_angle = rig.head_rotation_deg;
    const bool flip_x = !rig.facing_right;
    const float front_upper_angle = part_rotation_deg(assets.hero_rig.front_upper_arm, rig.front_arm.shoulder, rig.front_arm.elbow, flip_x);
    const float front_forearm_angle = part_rotation_deg(assets.hero_rig.front_forearm, rig.front_arm.elbow, rig.front_arm.hand, flip_x);
    const float back_upper_angle = part_rotation_deg(assets.hero_rig.back_upper_arm, rig.back_arm.shoulder, rig.back_arm.elbow, flip_x);
    const float back_forearm_angle = part_rotation_deg(assets.hero_rig.back_forearm, rig.back_arm.elbow, rig.back_arm.hand, flip_x);

    render_part(assets.hero_rig.sheet, assets.hero_rig.back_upper_arm, resolve_rig_anchor(rig, assets.hero_rig.back_upper_arm.anchor), back_upper_angle, rig_screen_scale, rig_screen_scale);
    render_part(assets.hero_rig.sheet, assets.hero_rig.back_forearm, resolve_rig_anchor(rig, assets.hero_rig.back_forearm.anchor), back_forearm_angle, rig_screen_scale, rig_screen_scale);
    render_part(assets.hero_rig.sheet, assets.hero_rig.torso, resolve_rig_anchor(rig, assets.hero_rig.torso.anchor), torso_angle, torso_scale_x, torso_scale_y);
    render_part(assets.hero_rig.sheet, assets.hero_rig.head, resolve_rig_anchor(rig, assets.hero_rig.head.anchor), head_angle, rig_screen_scale, rig_screen_scale);
    if (render_weapon_sprite) {
        render_weapon_with_rig(weapon_definition->texture, *weapon_definition, rig, alpha, camera);
    }
    render_part(assets.hero_rig.sheet, assets.hero_rig.front_upper_arm, resolve_rig_anchor(rig, assets.hero_rig.front_upper_arm.anchor), front_upper_angle, rig_screen_scale, rig_screen_scale);
    render_part(assets.hero_rig.sheet, assets.hero_rig.front_forearm, resolve_rig_anchor(rig, assets.hero_rig.front_forearm.anchor), front_forearm_angle, rig_screen_scale, rig_screen_scale);
}

void Renderer2D::render_aiming_rig_debug(const UpperBodyAimingRigState& rig, const Camera& camera)
{
    if (!rig.valid) {
        return;
    }

    const auto to_screen = [this, &camera](SDL_FPoint point) {
        return SDL_FPoint{
            std::round(world_to_screen_x(point.x, camera, presentation_rect_)),
            std::round(world_to_screen_y(point.y, camera, presentation_rect_))
        };
    };
    const auto draw_segment = [this](SDL_FPoint a, SDL_FPoint b, Uint8 r, Uint8 g, Uint8 bl, Uint8 alpha) {
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, r, g, bl, alpha);
        SDL_RenderLine(renderer_, a.x, a.y, b.x, b.y);
    };
    const auto draw_dashed_segment = [this](SDL_FPoint a, SDL_FPoint b, float dash_len, float gap_len, Uint8 r, Uint8 g, Uint8 bl, Uint8 alpha) {
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, r, g, bl, alpha);
        const SDL_FPoint delta{b.x - a.x, b.y - a.y};
        const float total_len = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (total_len <= 0.001f) {
            return;
        }
        const SDL_FPoint dir{delta.x / total_len, delta.y / total_len};
        float cursor = 0.0f;
        while (cursor < total_len) {
            const float start = cursor;
            const float end = std::min(total_len, cursor + dash_len);
            SDL_RenderLine(
                renderer_,
                a.x + dir.x * start,
                a.y + dir.y * start,
                a.x + dir.x * end,
                a.y + dir.y * end);
            cursor += dash_len + gap_len;
        }
    };
    const auto draw_marker = [this](SDL_FPoint p, float radius, Uint8 r, Uint8 g, Uint8 bl, Uint8 alpha) {
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, r, g, bl, alpha);
        const SDL_FRect rect{
            std::round(p.x - radius),
            std::round(p.y - radius),
            radius * 2.0f,
            radius * 2.0f
        };
        SDL_RenderFillRect(renderer_, &rect);
    };

    const SDL_FPoint pelvis = to_screen(rig.pelvis);
    const SDL_FPoint torso_base = to_screen(rig.torso_base);
    const SDL_FPoint torso_top = to_screen(rig.torso_top);
    const SDL_FPoint head = to_screen(rig.head_center);
    const SDL_FPoint front_shoulder = to_screen(rig.front_shoulder);
    const SDL_FPoint back_shoulder = to_screen(rig.back_shoulder);
    const SDL_FPoint front_elbow_hint = to_screen(rig.front_elbow_hint);
    const SDL_FPoint back_elbow_hint = to_screen(rig.back_elbow_hint);
    const SDL_FPoint rear_grip = to_screen(rig.rear_grip);
    const SDL_FPoint front_grip = to_screen(rig.front_grip);
    const SDL_FPoint muzzle = to_screen(rig.muzzle);
    const SDL_FPoint aim_target = to_screen(rig.aim_target);
    const SDL_FPoint front_elbow = to_screen(rig.front_arm.elbow);
    const SDL_FPoint back_elbow = to_screen(rig.back_arm.elbow);
    const float weapon_angle_rad = rig.weapon_rotation_deg * 3.1415926535f / 180.0f;
    const SDL_FPoint bore_forward{
        std::cos(weapon_angle_rad),
        -std::sin(weapon_angle_rad)
    };
    const SDL_FPoint bore_start{
        muzzle.x - bore_forward.x * 18.0f,
        muzzle.y - bore_forward.y * 18.0f
    };
    const SDL_FPoint bore_end{
        muzzle.x + bore_forward.x * 28.0f,
        muzzle.y + bore_forward.y * 28.0f
    };

    draw_segment(pelvis, torso_base, 182, 182, 200, 220);
    draw_segment(torso_base, torso_top, 220, 220, 235, 220);
    draw_segment(torso_top, head, 220, 220, 235, 200);

    draw_segment(front_shoulder, front_elbow, 255, 180, 90, 230);
    draw_segment(front_elbow, rear_grip, 255, 210, 100, 230);
    draw_segment(back_shoulder, back_elbow, 120, 180, 255, 230);
    draw_segment(back_elbow, front_grip, 130, 220, 255, 230);

    draw_segment(rear_grip, front_grip, 255, 235, 120, 220);
    draw_segment(front_grip, muzzle, 255, 160, 120, 150);
    draw_segment(bore_start, bore_end, 255, 90, 90, 240);
    draw_dashed_segment(muzzle, aim_target, 5.0f, 4.0f, 255, 120, 160, 210);

    draw_segment(front_shoulder, front_elbow_hint, 130, 90, 40, 120);
    draw_segment(back_shoulder, back_elbow_hint, 40, 90, 130, 120);

    draw_marker(head, std::max(2.0f, scale_world_length(kRigHeadRadius * 0.55f, camera, presentation_rect_)), 244, 233, 210, 220);
    draw_marker(front_shoulder, 2.0f, 255, 180, 90, 240);
    draw_marker(back_shoulder, 2.0f, 120, 180, 255, 240);
    draw_marker(front_elbow, 1.5f, 255, 180, 90, 200);
    draw_marker(back_elbow, 1.5f, 120, 180, 255, 200);
    draw_marker(front_elbow_hint, 1.5f, 130, 90, 40, 160);
    draw_marker(back_elbow_hint, 1.5f, 40, 90, 130, 160);
    draw_marker(rear_grip, 2.0f, 255, 230, 120, 255);
    draw_marker(front_grip, 2.0f, 255, 230, 120, 255);
    draw_marker(muzzle, 1.5f, 255, 110, 110, 255);
    draw_segment(
        SDL_FPoint{aim_target.x - 5.0f, aim_target.y},
        SDL_FPoint{aim_target.x + 5.0f, aim_target.y},
        140, 255, 180, 235);
    draw_segment(
        SDL_FPoint{aim_target.x, aim_target.y - 5.0f},
        SDL_FPoint{aim_target.x, aim_target.y + 5.0f},
        140, 255, 180, 235);
    draw_marker(aim_target, 1.0f, 140, 255, 180, 255);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void Renderer2D::render_bullet(const Bullet& bullet, const Camera& camera)
{
    if (!bullet.active) {
        return;
    }

    const float start_x = std::round(world_to_screen_x(bullet.x, camera, presentation_rect_));
    const float start_y = std::round(world_to_screen_y(bullet.y, camera, presentation_rect_));
    const float length = scale_world_length(18.0f, camera, presentation_rect_);
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
        SDL_FRect dst{
            std::round(world_to_screen_x(zombie.x, camera, presentation_rect_)),
            world_to_screen_y(zombie.y, camera, presentation_rect_),
            scale_world_length(kZombieWidth, camera, presentation_rect_),
            scale_world_length(kZombieHeight, camera, presentation_rect_)
        };
        const SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
        const SDL_FlipMode flip = zombie.walking_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
        SDL_RenderTextureRotated(renderer_, texture.get(), &src, &dst, 0.0, &center, flip);
    } else {
        const SDL_FRect src{sprites::zombie_corpse_frame()};
        const SDL_FRect dst{
            std::round(world_to_screen_x(zombie.x + 7.5f, camera, presentation_rect_)),
            world_to_screen_y(zombie.y + kZombieCorpseRenderYOffset, camera, presentation_rect_),
            scale_world_length(17.0f, camera, presentation_rect_),
            scale_world_length(32.0f, camera, presentation_rect_)
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
