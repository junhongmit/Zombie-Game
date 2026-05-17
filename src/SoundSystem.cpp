#include "SoundSystem.h"

#include "AssetPaths.h"
#include <SDL3/SDL_audio.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace zg {

SoundSystem::SoundSystem()
{
    mix_spec_.format = SDL_AUDIO_F32;
    mix_spec_.channels = 2;
    mix_spec_.freq = 48000;
}

SoundSystem::~SoundSystem()
{
    shutdown();
}

bool SoundSystem::init()
{
    mutex_ = SDL_CreateMutex();
    if (mutex_ == nullptr) {
        std::fprintf(stderr, "SDL_CreateMutex failed: %s\n", SDL_GetError());
        return false;
    }

    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &mix_spec_, &SoundSystem::stream_callback, this);
    if (stream_ == nullptr) {
        std::fprintf(stderr, "SDL_OpenAudioDeviceStream failed: %s\n", SDL_GetError());
        shutdown();
        return false;
    }

    if (!SDL_ResumeAudioStreamDevice(stream_)) {
        std::fprintf(stderr, "SDL_ResumeAudioStreamDevice failed: %s\n", SDL_GetError());
        shutdown();
        return false;
    }

    return true;
}

void SoundSystem::shutdown()
{
    if (stream_ != nullptr) {
        SDL_DestroyAudioStream(stream_);
        stream_ = nullptr;
    }
    if (mutex_ != nullptr) {
        SDL_DestroyMutex(mutex_);
        mutex_ = nullptr;
    }

    for (LoadedSound& sound : sounds_) {
        sound.samples.clear();
        sound.valid = false;
    }
}

bool SoundSystem::load_defaults()
{
    return load_sound(SoundId::GlockShot, "music/sound/weapons/Glock/glock18-2.wav") &&
        load_sound(SoundId::GrenadeThrow, "music/gre1.wav") &&
        load_sound(SoundId::Explosion1, "music/sound/Explosions/exp1.wav") &&
        load_sound(SoundId::Explosion2, "music/sound/Explosions/exp2.wav") &&
        load_sound(SoundId::Explosion3, "music/sound/Explosions/exp3.wav") &&
        load_sound(SoundId::Explosion4, "music/sound/Explosions/exp4.wav");
}

void SoundSystem::play(SoundId id, float volume)
{
    const LoadedSound& sound = sounds_[static_cast<int>(id)];
    if (!sound.valid || sound.samples.empty() || mutex_ == nullptr) {
        return;
    }

    SDL_LockMutex(mutex_);
    Voice& voice = voices_[next_voice_];
    next_voice_ = (next_voice_ + 1) % kVoiceCount;
    voice.data = sound.samples.data();
    voice.sample_count = static_cast<int>(sound.samples.size());
    voice.cursor = 0;
    voice.volume = volume;
    voice.active = true;
    SDL_UnlockMutex(mutex_);
}

bool SoundSystem::preload_file(const char* path)
{
    return get_or_load_file(path) != nullptr;
}

void SoundSystem::play_file(const char* path, float volume)
{
    const LoadedSound* sound = get_or_load_file(path);
    if (sound == nullptr || sound->samples.empty() || mutex_ == nullptr) {
        return;
    }

    SDL_LockMutex(mutex_);
    Voice& voice = voices_[next_voice_];
    next_voice_ = (next_voice_ + 1) % kVoiceCount;
    voice.data = sound->samples.data();
    voice.sample_count = static_cast<int>(sound->samples.size());
    voice.cursor = 0;
    voice.volume = volume;
    voice.active = true;
    SDL_UnlockMutex(mutex_);
}

void SoundSystem::play_random_explosion()
{
    const int pick = std::rand() % 4;
    play(static_cast<SoundId>(static_cast<int>(SoundId::Explosion1) + pick), 0.95f);
}

void SDLCALL SoundSystem::stream_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int)
{
    SoundSystem* self = static_cast<SoundSystem*>(userdata);
    if (self != nullptr) {
        self->mix_into_stream(stream, additional_amount);
    }
}

void SoundSystem::mix_into_stream(SDL_AudioStream* stream, int additional_amount)
{
    if (additional_amount <= 0) {
        return;
    }

    std::vector<float> mix_buffer(static_cast<size_t>(additional_amount) / sizeof(float), 0.0f);

    if (mutex_ != nullptr) {
        SDL_LockMutex(mutex_);
        for (Voice& voice : voices_) {
            if (!voice.active || voice.data == nullptr || voice.cursor >= voice.sample_count) {
                voice.active = false;
                continue;
            }

            const int remaining = voice.sample_count - voice.cursor;
            const int count = std::min(static_cast<int>(mix_buffer.size()), remaining);
            for (int i = 0; i < count; ++i) {
                mix_buffer[static_cast<size_t>(i)] += voice.data[voice.cursor + i] * voice.volume;
            }
            voice.cursor += count;
            if (voice.cursor >= voice.sample_count) {
                voice.active = false;
            }
        }
        SDL_UnlockMutex(mutex_);
    }

    for (float& sample : mix_buffer) {
        sample = std::max(-1.0f, std::min(1.0f, sample));
    }

    SDL_PutAudioStreamData(stream, mix_buffer.data(), additional_amount);
}

bool SoundSystem::load_sound(SoundId id, const char* path)
{
    const std::string resolved = resolve_asset_path(path);
    SDL_AudioSpec src_spec{};
    Uint8* src_buffer = nullptr;
    Uint32 src_length = 0;
    if (!SDL_LoadWAV(resolved.c_str(), &src_spec, &src_buffer, &src_length)) {
        std::fprintf(stderr, "SDL_LoadWAV failed for %s: %s\n", resolved.c_str(), SDL_GetError());
        return false;
    }

    Uint8* converted_buffer = nullptr;
    int converted_length = 0;
    const bool converted = SDL_ConvertAudioSamples(
        &src_spec,
        src_buffer,
        static_cast<int>(src_length),
        &mix_spec_,
        &converted_buffer,
        &converted_length);
    SDL_free(src_buffer);

    if (!converted) {
        std::fprintf(stderr, "SDL_ConvertAudioSamples failed for %s: %s\n", resolved.c_str(), SDL_GetError());
        return false;
    }

    LoadedSound& sound = sounds_[static_cast<int>(id)];
    sound.samples.resize(static_cast<size_t>(converted_length) / sizeof(float));
    std::memcpy(sound.samples.data(), converted_buffer, static_cast<size_t>(converted_length));
    sound.valid = true;
    SDL_free(converted_buffer);
    return true;
}

const SoundSystem::LoadedSound* SoundSystem::get_or_load_file(const char* path)
{
    if (path == nullptr || path[0] == '\0') {
        return nullptr;
    }

    const std::string key = resolve_asset_path(path);
    const auto found = file_sounds_.find(key);
    if (found != file_sounds_.end()) {
        return &found->second;
    }

    SDL_AudioSpec src_spec{};
    Uint8* src_buffer = nullptr;
    Uint32 src_length = 0;
    if (!SDL_LoadWAV(key.c_str(), &src_spec, &src_buffer, &src_length)) {
        std::fprintf(stderr, "SDL_LoadWAV failed for %s: %s\n", key.c_str(), SDL_GetError());
        return nullptr;
    }

    Uint8* converted_buffer = nullptr;
    int converted_length = 0;
    const bool converted = SDL_ConvertAudioSamples(
        &src_spec,
        src_buffer,
        static_cast<int>(src_length),
        &mix_spec_,
        &converted_buffer,
        &converted_length);
    SDL_free(src_buffer);
    if (!converted) {
        std::fprintf(stderr, "SDL_ConvertAudioSamples failed for %s: %s\n", key.c_str(), SDL_GetError());
        return nullptr;
    }

    LoadedSound sound;
    sound.samples.resize(static_cast<size_t>(converted_length) / sizeof(float));
    std::memcpy(sound.samples.data(), converted_buffer, static_cast<size_t>(converted_length));
    sound.valid = true;
    SDL_free(converted_buffer);

    const auto inserted = file_sounds_.emplace(key, std::move(sound));
    return &inserted.first->second;
}

} // namespace zg
