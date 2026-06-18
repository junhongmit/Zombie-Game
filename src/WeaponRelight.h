#pragma once

#include <vector>

struct SDL_Renderer;

namespace zg {

class Texture;

struct WorkbenchShadowPlacement {
    float anchor_x = 0.0f;
    float anchor_y = 0.0f;
};

struct WorkbenchRelightBakeResult {
    bool valid = false;
    int lit_width = 0;
    int lit_height = 0;
    std::vector<unsigned char> lit_rgba;
    int shadow_width = 0;
    int shadow_height = 0;
    std::vector<unsigned char> shadow_rgba;
    WorkbenchShadowPlacement shadow_placement;
};

bool bake_workbench_relight_assets(
    const char* albedo_path,
    const char* depth_path,
    WorkbenchRelightBakeResult* out_result);

bool upload_workbench_relight_assets(
    SDL_Renderer* renderer,
    const WorkbenchRelightBakeResult& baked,
    Texture* out_lit_texture,
    Texture* out_shadow_texture,
    WorkbenchShadowPlacement* out_shadow_placement);

bool build_workbench_relight_assets(
    SDL_Renderer* renderer,
    const char* albedo_path,
    const char* depth_path,
    Texture* out_lit_texture,
    Texture* out_shadow_texture,
    WorkbenchShadowPlacement* out_shadow_placement);

} // namespace zg
