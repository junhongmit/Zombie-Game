#include "ZombieDirector.h"

#include "Constants.h"
#include "MathUtil.h"
#include "Player.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdlib>

namespace zg {

namespace {

float random_spawn_interval()
{
    const float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return kZombieSpawnIntervalMin + (kZombieSpawnIntervalMax - kZombieSpawnIntervalMin) * t;
}

const ZombieDirector::StairInfo* stair_info_for_floor_step(
    const ZombieDirector::StairInfo& red,
    const ZombieDirector::StairInfo& green,
    const ZombieDirector::StairInfo& blue,
    int current_floor,
    int target_floor)
{
    if (target_floor > current_floor) {
        if (current_floor == 1) {
            return &red;
        }
        if (current_floor == 2) {
            return &green;
        }
        if (current_floor == 3) {
            return &blue;
        }
    } else if (target_floor < current_floor) {
        if (current_floor == 4) {
            return &blue;
        }
        if (current_floor == 3) {
            return &green;
        }
        if (current_floor == 2) {
            return &red;
        }
    }
    return nullptr;
}

} // namespace

void ZombieDirector::init(const CollisionMap& collision_map)
{
    find_stair_centers(collision_map);
    spawn_timer_ = random_spawn_interval();
    initialized_ = true;
}

void ZombieDirector::update(const CollisionMap& collision_map, const Player& player, Zombie* zombies, int zombie_count, float dt)
{
    if (!initialized_) {
        init(collision_map);
    }

    alive_count_ = 0;
    for (int i = 0; i < zombie_count; ++i) {
        if (zombies[i].active && zombies[i].alive) {
            ++alive_count_;
        }
    }

    spawn_timer_ -= dt;
    if (spawn_timer_ <= 0.0f) {
        spawn_if_needed(zombies, zombie_count);
        spawn_timer_ = random_spawn_interval();
    }

    update_ai(collision_map, player, zombies, zombie_count);
}

float ZombieDirector::move_axis_for(int zombie_index) const
{
    if (zombie_index < 0 || zombie_index >= kZombiePoolSize) {
        return 0.0f;
    }
    return move_axes_[zombie_index];
}

int ZombieDirector::wave() const
{
    return wave_;
}

int ZombieDirector::alive_count() const
{
    return alive_count_;
}

void ZombieDirector::find_stair_centers(const CollisionMap& collision_map)
{
    red_stair_.trigger = TriggerType::StairRed;
    red_stair_.x = 0.0f;
    red_stair_.found = false;
    green_stair_.trigger = TriggerType::StairGreen;
    green_stair_.x = 0.0f;
    green_stair_.found = false;
    blue_stair_.trigger = TriggerType::StairBlue;
    blue_stair_.x = 0.0f;
    blue_stair_.found = false;

    int sums[3] = {0, 0, 0};
    int counts[3] = {0, 0, 0};

    for (int y = 0; y < collision_map.height(); ++y) {
        for (int x = 0; x < collision_map.width(); ++x) {
            const TriggerType trigger = collision_map.trigger_at(x, y);
            if (trigger == TriggerType::None) {
                continue;
            }
            const int index = trigger == TriggerType::StairRed ? 0 : (trigger == TriggerType::StairGreen ? 1 : 2);
            sums[index] += x;
            counts[index] += 1;
        }
    }

    if (counts[0] > 0) {
        red_stair_.x = static_cast<float>(sums[0]) / static_cast<float>(counts[0]);
        red_stair_.found = true;
    }
    if (counts[1] > 0) {
        green_stair_.x = static_cast<float>(sums[1]) / static_cast<float>(counts[1]);
        green_stair_.found = true;
    }
    if (counts[2] > 0) {
        blue_stair_.x = static_cast<float>(sums[2]) / static_cast<float>(counts[2]);
        blue_stair_.found = true;
    }
}

void ZombieDirector::spawn_if_needed(Zombie* zombies, int zombie_count)
{
    const int target_alive = kWaveBaseConcurrent + (wave_ - 1) * kWaveConcurrentPerWave;
    if (alive_count_ >= target_alive) {
        return;
    }

    spawn_one(zombies, zombie_count, (std::rand() % 2) == 0);
    ++alive_count_;
}

void ZombieDirector::spawn_one(Zombie* zombies, int zombie_count, bool from_left)
{
    for (int i = 0; i < zombie_count; ++i) {
        Zombie& zombie = zombies[i];
        if (zombie.active) {
            continue;
        }

        zombie = Zombie(from_left ? 0.0f : (kWorldWidth - kZombieWidth), kFloor1Y, 18.0f + static_cast<float>(std::rand() % 8), !from_left, std::rand() % 24);
        zombie.active = true;
        zombie.alive = true;
        zombie.airborne = false;
        zombie.hp = 100;
        zombie.vx = 0.0f;
        zombie.vy = 0.0f;
        zombie.walking_right = !from_left;
        return;
    }
}

void ZombieDirector::update_ai(const CollisionMap& collision_map, const Player& player, Zombie* zombies, int zombie_count)
{
    for (int i = 0; i < kZombiePoolSize; ++i) {
        move_axes_[i] = 0.0f;
    }

    for (int i = 0; i < zombie_count && i < kZombiePoolSize; ++i) {
        Zombie& zombie = zombies[i];
        if (!zombie.active || !zombie.alive) {
            continue;
        }

        move_axes_[i] = compute_move_axis(collision_map, player, &zombie);
    }
}

float ZombieDirector::compute_move_axis(const CollisionMap& collision_map, const Player& player, Zombie* zombie) const
{
    if (zombie->airborne) {
        return 0.0f;
    }

    const int zombie_floor = floor_index_from_y(zombie->y);
    const int target_floor = floor_index_from_y(player.y);
    const float zombie_center_x = zombie->x + 9.0f;
    const float player_center_x = player.x + 9.0f;

    if (zombie_floor != target_floor) {
        if (try_use_stairs(collision_map, zombie, target_floor)) {
            return 0.0f;
        }

        const StairInfo* stair = stair_info_for_floor_step(red_stair_, green_stair_, blue_stair_, zombie_floor, target_floor);
        if (stair == nullptr || !stair->found) {
            return 0.0f;
        }

        const float dx = stair->x - zombie_center_x;
        if (std::fabs(dx) <= 3.0f) {
            return 0.0f;
        }
        return dx > 0.0f ? 1.0f : -1.0f;
    }

    const float dx = player_center_x - zombie_center_x;
    if (std::fabs(dx) <= 2.0f) {
        return 0.0f;
    }
    return dx > 0.0f ? 1.0f : -1.0f;
}

bool ZombieDirector::try_use_stairs(const CollisionMap& collision_map, Zombie* zombie, int target_floor) const
{
    const int trigger_x = static_cast<int>(std::round(zombie->x + 8.0f));
    const int trigger_y = static_cast<int>(std::round(zombie->y + 16.0f));
    const TriggerType trigger = collision_map.trigger_at(trigger_x, trigger_y);
    if (trigger == TriggerType::None) {
        return false;
    }

    const int floor = floor_index_from_y(zombie->y);
    int destination_floor = floor;
    switch (trigger) {
    case TriggerType::StairRed:
        if (target_floor > floor && floor == 1) {
            destination_floor = 2;
        } else if (target_floor < floor && floor == 2) {
            destination_floor = 1;
        }
        break;
    case TriggerType::StairGreen:
        if (target_floor > floor && floor == 2) {
            destination_floor = 3;
        } else if (target_floor < floor && floor == 3) {
            destination_floor = 2;
        }
        break;
    case TriggerType::StairBlue:
        if (target_floor > floor && floor == 3) {
            destination_floor = 4;
        } else if (target_floor < floor && floor == 4) {
            destination_floor = 3;
        }
        break;
    case TriggerType::None:
        break;
    }

    if (destination_floor == floor) {
        return false;
    }

    zombie->y = floor_y_from_index(destination_floor);
    zombie->vy = 0.0f;
    zombie->airborne = false;
    zombie->walk_frame = 0;
    zombie->walk_frame_distance = 0.0f;
    return true;
}

int ZombieDirector::floor_index_from_y(float y)
{
    if (y <= 200.0f) {
        return 4;
    }
    if (y <= 280.0f) {
        return 3;
    }
    if (y <= 360.0f) {
        return 2;
    }
    return 1;
}

float ZombieDirector::floor_y_from_index(int floor)
{
    switch (floor) {
    case 4:
        return kFloor4Y;
    case 3:
        return kFloor3Y;
    case 2:
        return kFloor2Y;
    case 1:
    default:
        return kFloor1Y;
    }
}

} // namespace zg
