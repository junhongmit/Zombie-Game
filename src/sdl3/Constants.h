#pragma once

namespace zg {

constexpr int kLogicalWidth = 640;
constexpr int kLogicalHeight = 480;
constexpr float kWorldWidth = 1072.0f;
constexpr float kPlayerWidth = 18.0f;
constexpr float kPlayerHeight = 33.0f;
constexpr float kFloor1Y = 400.0f;
constexpr float kFloor2Y = 320.0f;
constexpr float kFloor3Y = 240.0f;
constexpr float kFloor4Y = 160.0f;
constexpr float kGroundY = 480.0f - 48.0f - kPlayerHeight;
constexpr float kPlayerMoveSpeed = 90.0f;
constexpr float kPlayerJumpSpeed = 240.0f;
constexpr float kGravity = 560.0f;
constexpr float kTerminalVelocity = 360.0f;
constexpr float kWalkPixelsPerFrame = 3.5f;
constexpr float kBulletSpeed = 760.0f;
constexpr float kBulletLifetimeSeconds = 1.1f;
constexpr float kGlockFireIntervalSeconds = 0.10f;
constexpr float kBulletTraceStep = 3.0f;
constexpr int kBodyShotDamage = 20;
constexpr int kHeadShotDamage = 50;
constexpr float kZombieHeadHeight = 11.0f;
constexpr float kZombieWidth = 18.0f;
constexpr float kZombieHeight = 32.0f;
constexpr float kZombieCorpseWidth = 32.0f;
constexpr float kZombieCorpseHeight = 18.0f;
constexpr float kZombieCorpseYOffset = 6.0f;
constexpr float kZombieCorpseSpinSpeed = 420.0f;
constexpr float kZombieCorpseDriftSpeed = 26.0f;
constexpr float kZombieCorpseFadeDelay = 2.5f;
constexpr float kZombieCorpseFadeSeconds = 1.2f;

} // namespace zg
