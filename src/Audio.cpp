#include "Audio.h"
#include "Music.h"

#include <miniaudio.h>

#include <iostream>

struct Audio::Impl {
    ma_device device{};
};

struct AudioCallback {
    static void fill(ma_device* dev, void* out, const void*, ma_uint32 frames) {
        Audio* a = static_cast<Audio*>(dev->pUserData);
        float* o = static_cast<float*>(out);
        if (!a->playing_) {
            for (ma_uint32 i = 0; i < frames * 2; ++i) o[i] = 0.0f;
            return;
        }
        long long start = a->frame_.load();
        float vol = a->muted_ ? 0.0f : 0.8f;
        for (ma_uint32 i = 0; i < frames; ++i) {
            long long n = start + i;
            float l, r;
            music::sample((double)n / music::kSampleRate, n, l, r);
            o[2 * i] = l * vol;
            o[2 * i + 1] = r * vol;
        }
        a->frame_.store(start + frames);
    }
};

Audio::Audio() : impl_(std::make_unique<Impl>()) {}
Audio::~Audio() { shutdown(); }

bool Audio::init() {
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 2;
    cfg.sampleRate = music::kSampleRate;
    cfg.periodSizeInMilliseconds = 10;
    cfg.dataCallback = AudioCallback::fill;
    cfg.pUserData = this;
    if (ma_device_init(nullptr, &cfg, &impl_->device) != MA_SUCCESS) {
        std::cerr << "[audio] no output device - the show runs silently\n";
        return false;
    }
    if (ma_device_start(&impl_->device) != MA_SUCCESS) {
        ma_device_uninit(&impl_->device);
        std::cerr << "[audio] could not start the output device\n";
        return false;
    }
    ok_ = true;
    std::cout << "[audio] " << impl_->device.playback.name << ", " << music::kSampleRate << " Hz" << std::endl;
    return true;
}

void Audio::shutdown() {
    if (!ok_) return;
    ma_device_uninit(&impl_->device);
    ok_ = false;
}

void Audio::seek(double seconds) { frame_ = (long long)(seconds * music::kSampleRate); }

double Audio::time() const {
    return (double)frame_.load() / music::kSampleRate - 0.010;
}
