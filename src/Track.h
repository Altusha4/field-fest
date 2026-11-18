#pragma once
#include "Music.h"

#include <cstdint>
#include <string>
#include <vector>

class Track {
public:
    bool load(const std::string& path);
    const std::string& title() const { return title_; }
    double bpm() const { return bpm_; }
    double duration() const { return (double)frames_ / music::kSampleRate; }
    void sample(long long n, float& l, float& r) const;
    music::Levels levels(double t) const;
    music::Section section(double t) const;
    const std::vector<music::Note>& notes() const { return notes_; }

private:
    static constexpr int kHop = 512;
    static constexpr double kFps = (double)music::kSampleRate / kHop;
    void analyse();
    int hopAt(double t) const;

    std::string title_;
    std::vector<float> pcm_;
    long long frames_ = 0;
    double bpm_ = 120.0, offset_ = 0.0;
    std::vector<float> low_, high_, loud_;
    std::vector<uint8_t> section_;
    std::vector<double> kicks_;
    std::vector<music::Note> notes_;
};
