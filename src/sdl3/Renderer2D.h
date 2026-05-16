#pragma once

#include <SDL3/SDL.h>

namespace zg {

struct Assets;
struct Camera;
struct Player;
class Texture;
struct Zombie;

class Renderer2D {
public:
    explicit Renderer2D(SDL_Renderer* renderer);

    void begin_frame();
    void render_scene(const Assets& assets, const Player& player, const Zombie* zombies, int zombie_count, const Camera& camera);
    void end_frame();

private:
    void render_fullscreen(const Texture& texture);
    void render_scrolling_layer(const Texture& texture, float camera_x, float parallax, float y, float height);
    void render_world_layer(const Texture& texture, float camera_x);
    void render_aim_line(const Player& player, const Camera& camera);
    void render_weapon(const Texture& weapon, const Player& player, const Camera& camera);
    void render_player(const Texture& hero, const Player& player, const Camera& camera);
    void render_zombie(const Texture& texture, const Zombie& zombie, const Camera& camera);

    SDL_Renderer* renderer_;
};

} // namespace zg
