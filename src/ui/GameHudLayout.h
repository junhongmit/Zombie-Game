#pragma once

#include "LayoutTypes.h"

#include <SDL3/SDL.h>

namespace zg {

class GameHudLayout {
public:
    bool load(const char* asset_path);

    SDL_FRect portrait_panel_rect() const;
    SDL_FRect top_panel_rect(int index) const;
    SDL_FRect time_panel_rect() const;
    SDL_FRect objectives_panel_rect() const;
    SDL_FRect warning_panel_rect() const;
    SDL_FRect mode_button_rect(int index) const;
    SDL_FRect tool_slot_rect(int index) const;
    SDL_FRect survivor_panel_rect(int index) const;

private:
    SDL_FRect to_logical_rect(const NormalizedRect& rect) const;
    float normalized_x(float value) const;
    float normalized_y(float value) const;

    NormalizedRect portrait_panel_{0.009375f, 0.02037f, 0.18f, 0.1f};
    NormalizedRect top_panels_[4] = {
        NormalizedRect{0.209375f, 0.02037f, 0.129167f, 0.053704f},
        NormalizedRect{0.351042f, 0.02037f, 0.088542f, 0.053704f},
        NormalizedRect{0.448958f, 0.02037f, 0.078125f, 0.053704f},
        NormalizedRect{0.536458f, 0.02037f, 0.097917f, 0.053704f},
    };
    NormalizedRect time_panel_{0.805208f, 0.02037f, 0.175f, 0.053704f};
    NormalizedRect objectives_panel_{0.805208f, 0.096296f, 0.175f, 0.25f};
    NormalizedRect warning_panel_{0.805208f, 0.368519f, 0.175f, 0.157407f};
    NormalizedRect mode_button_start_{0.017708f, 0.853704f, 0.05625f, 0.048148f};
    float mode_button_gap_ = 0.008333f;
    int mode_button_count_ = 4;
    NormalizedRect tool_slot_start_{0.017708f, 0.907407f, 0.05625f, 0.07963f};
    float tool_slot_gap_ = 0.008333f;
    int tool_slot_count_ = 9;
    NormalizedRect survivor_start_{0.71875f, 0.837037f, 0.08125f, 0.151852f};
    float survivor_gap_ = 0.008333f;
    int survivor_count_ = 3;
};

} // namespace zg
