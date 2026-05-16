#pragma once

#include <SDL3/SDL.h>

namespace zg {

enum class TriggerType {
    None,
    StairRed,
    StairGreen,
    StairBlue
};

class CollisionMap {
public:
    CollisionMap();
    ~CollisionMap();

    CollisionMap(const CollisionMap&) = delete;
    CollisionMap& operator=(const CollisionMap&) = delete;

    CollisionMap(CollisionMap&& other) noexcept;
    CollisionMap& operator=(CollisionMap&& other) noexcept;

    bool load(const char* path);
    void reset();

    bool is_solid(int x, int y) const;
    TriggerType trigger_at(int x, int y) const;
    int width() const;
    int height() const;

private:
    SDL_Surface* surface_;
};

} // namespace zg
