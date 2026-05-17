#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace zg {

struct Assets;
struct Bullet;
struct Camera;
class EffectsSystem;
struct Explosion;
struct Grenade;
struct Player;
class Texture;
struct WeaponDefinition;
struct WeaponState;
struct Zombie;

class Renderer2D {
public:
    explicit Renderer2D(SDL_Renderer* renderer);
    ~Renderer2D();

    void begin_frame();
    void render_scene(
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
        const Camera& camera);
    void end_frame();

private:
    void render_fullscreen(const Texture& texture);
    void render_scrolling_layer(const Texture& texture, float camera_x, float parallax, float y, float height);
    void render_world_layer(const Texture& texture, float camera_x);
    void render_weapon_hud(const Texture& bullet_icon, const Player& player, const WeaponDefinition* weapon_definition, const WeaponState& weapon, const Camera& camera);
    void render_hud_text(const char* text, float x, float y);
    void render_blood_particle(float x, float y, const Camera& camera);
    void render_explosion(const Texture* frames, const Explosion& explosion, const Camera& camera);
    void render_grenade(const Grenade& grenade, const Camera& camera);
    void render_smoke_particle(const Texture& texture, float x, float y, float size, float alpha, const Camera& camera);
    void render_bullet(const Bullet& bullet, const Camera& camera);
    void render_weapon(const Texture& weapon, const WeaponDefinition& definition, const Player& player, const Camera& camera);
    void render_player(const Texture& hero, const Player& player, const Camera& camera);
    void render_zombie(const Texture& texture, const Zombie& zombie, const Camera& camera);

    SDL_Renderer* renderer_;
    TTF_Font* hud_font_ = nullptr;
};

} // namespace zg
