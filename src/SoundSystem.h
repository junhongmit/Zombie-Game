#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace zg {

enum class SoundId {
    GlockShot,
    GrenadeThrow,
    Explosion1,
    Explosion2,
    Explosion3,
    Explosion4,
    Count
};

class SoundSystem {
public:
    SoundSystem();
    ~SoundSystem();

    bool init();
    void shutdown();
    bool load_defaults();
    bool preload_file(const char* path);
    void play_file(const char* path, float volume = 1.0f);
    void play(SoundId id, float volume = 1.0f);
    void play_random_explosion();

private:
    struct LoadedSound {
        std::vector<float> samples;
        bool valid = false;
    };

    struct Voice {
        const float* data = nullptr;
        int sample_count = 0;
        int cursor = 0;
        float volume = 1.0f;
        bool active = false;
    };

    static void SDLCALL stream_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
    void mix_into_stream(SDL_AudioStream* stream, int additional_amount);
    bool load_sound(SoundId id, const char* path);
    const LoadedSound* get_or_load_file(const char* path);

    static constexpr int kVoiceCount = 16;

    SDL_AudioSpec mix_spec_{};
    SDL_AudioStream* stream_ = nullptr;
    SDL_Mutex* mutex_ = nullptr;
    std::array<LoadedSound, static_cast<int>(SoundId::Count)> sounds_{};
    std::unordered_map<std::string, LoadedSound> file_sounds_;
    std::array<Voice, kVoiceCount> voices_{};
    int next_voice_ = 0;
};

} // namespace zg
