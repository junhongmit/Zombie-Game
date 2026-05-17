#pragma once

namespace zg {

class CollisionMap;

class HumanoidBody {
public:
    HumanoidBody(float width, float height, int probe_left, int probe_right, int foot_probe_start, int foot_probe_end);

    float width() const;
    float height() const;
    bool overlaps_solid(const CollisionMap& collision_map, float x, float y) const;
    bool has_floor_support(const CollisionMap& collision_map, float x, float y) const;
    bool collides_head(const CollisionMap& collision_map, float x, float y) const;
    bool collides_left(const CollisionMap& collision_map, float x, float y) const;
    bool collides_right(const CollisionMap& collision_map, float x, float y) const;
    float snap_to_floor(const CollisionMap& collision_map, float x, float y, float world_height_limit) const;
    void move_horizontally(const CollisionMap& collision_map, float axis, float speed, float dt, float* x, float y, float world_width_limit) const;
    void move_by_velocity(const CollisionMap& collision_map, float dt, float* x, float* vx, float y, float world_width_limit) const;
    void move_vertically(
        const CollisionMap& collision_map,
        float gravity,
        float min_velocity,
        float max_velocity,
        float dt,
        float x,
        float* y,
        float* vy,
        bool* airborne,
        float world_height_limit) const;

private:
    float width_;
    float height_;
    int probe_left_;
    int probe_right_;
    int foot_probe_start_;
    int foot_probe_end_;
};

} // namespace zg
