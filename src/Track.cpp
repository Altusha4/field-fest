#include "Track.h"
#include "MathUtil.h"

#include <miniaudio.h>

#include <algorithm>
#include <complex>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <numeric>

namespace {

float percentile(std::vector<float> v, float q) {
    if (v.empty()) return 1.0f;
    size_t k = std::min(v.size() - 1, (size_t)(q * (v.size() - 1)));
    std::nth_element(v.begin(), v.begin() + k, v.end());
    return std::max(v[k], 1e-9f);
}

void normalise(std::vector<float>& v) {
    float p = percentile(v, 0.95f);
    for (float& x : v) x = std::min(1.0f, x / p);
}

std::vector<float> smooth(const std::vector<float>& v, int r) {
    std::vector<double> sum(v.size() + 1, 0.0);
    for (size_t i = 0; i < v.size(); ++i) sum[i + 1] = sum[i] + v[i];
    std::vector<float> out(v.size());
    for (int i = 0; i < (int)v.size(); ++i) {
        int a = std::max(0, i - r), b = std::min((int)v.size(), i + r + 1);
        out[i] = (float)((sum[b] - sum[a]) / (b - a));
    }
    return out;
}

}

bool Track::load(const std::string& path) {
    namespace fs = std::filesystem;
    fs::path p = fs::u8path(path);
    title_ = p.stem().u8string();

    ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 2, music::kSampleRate);
    ma_decoder dec;
#ifdef _WIN32
    ma_result res = ma_decoder_init_file_w(p.wstring().c_str(), &cfg, &dec);
#else
    ma_result res = ma_decoder_init_file(path.c_str(), &cfg, &dec);
#endif
    if (res != MA_SUCCESS) {
        std::cerr << "[track] cannot decode " << path << "\n";
        return false;
    }
    pcm_.clear();
    std::vector<float> buf(4096 * 2);
    for (;;) {
        ma_uint64 got = 0;
        ma_decoder_read_pcm_frames(&dec, buf.data(), 4096, &got);
        if (got == 0) break;
        pcm_.insert(pcm_.end(), buf.begin(), buf.begin() + (size_t)got * 2);
    }
    ma_decoder_uninit(&dec);
    frames_ = (long long)pcm_.size() / 2;
    if (frames_ < music::kSampleRate * 10) {
        std::cerr << "[track] " << path << " is too short\n";
        return false;
    }
    analyse();
    std::cout << "[track] " << title_ << ": " << (int)duration() << " s, " << (int)std::round(bpm_) << " BPM, "
              << notes_.size() << " notes\n";
    return true;
}

static void fft(std::vector<std::complex<float>>& a) {
    const size_t n = a.size();
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1) {
        float ang = -mathx::kTau / (float)len;
        std::complex<float> wl(std::cos(ang), std::sin(ang));
        for (size_t i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k) {
                std::complex<float> u = a[i + k], v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

void Track::analyse() {
    const int hops = (int)(frames_ / kHop);
    const int N = 1024;
    const int lowBins = 5;
    const int highBin = 64;
    std::vector<float> win(N);
    for (int i = 0; i < N; ++i) win[i] = 0.5f - 0.5f * std::cos(mathx::kTau * i / (N - 1));
    std::vector<std::complex<float>> X(N);
    std::vector<float> prev(N / 2, 0.0f), spec(N / 2);
    std::vector<float> eLow(hops), eHigh(hops), eAll(hops);
    std::vector<float> onLow(hops, 0.0f), onHigh(hops, 0.0f), onset(hops, 0.0f);
    for (int h = 0; h < hops; ++h) {
        long long start = (long long)h * kHop - N / 2;
        double sa = 0.0;
        for (int i = 0; i < N; ++i) {
            long long n = start + i;
            float m = (n >= 0 && n < frames_) ? 0.5f * (pcm_[2 * n] + pcm_[2 * n + 1]) : 0.0f;
            X[i] = std::complex<float>(m * win[i], 0.0f);
            sa += m * m;
        }
        fft(X);
        float fl = 0, fh = 0, fa = 0, el = 0, eh = 0;
        for (int k = 1; k < N / 2; ++k) {
            float mag = std::abs(X[k]);
            spec[k] = std::log(1.0f + 100.0f * mag);
            float d = std::max(0.0f, spec[k] - prev[k]);
            fa += d;
            if (k <= lowBins) {
                fl += d;
                el += mag * mag;
            } else if (k >= highBin) {
                fh += d;
                eh += mag * mag;
            }
        }
        std::swap(prev, spec);
        eLow[h] = el;
        eHigh[h] = eh;
        eAll[h] = (float)(sa / N);
        onLow[h] = fl;
        onHigh[h] = fh / (N / 2 - highBin);
        onset[h] = fa / (N / 2);
    }
    auto peaks = [&](std::vector<float>& v) {
        std::vector<float> m = smooth(v, (int)(kFps * 0.25));
        for (size_t i = 0; i < v.size(); ++i) v[i] = std::max(0.0f, v[i] - m[i]);
    };
    peaks(onset);
    peaks(onLow);
    peaks(onHigh);

    std::vector<float> strength(hops);
    for (int h = 0; h < hops; ++h) strength[h] = onset[h] + 0.5f * onLow[h];
    float thr = percentile(strength, 0.75f);
    std::vector<std::pair<int, float>> hits;
    double total = 0.0;
    for (int h = 0; h < hops; ++h)
        if (strength[h] > thr) {
            hits.push_back({h, strength[h]});
            total += strength[h];
        }
    double bestRatio = 0.0, bestBpm = 120.0, bestPhase = 0.0;
    std::vector<double> hist;
    for (double bpm = 60.0; bpm <= 200.0; bpm += 0.01) {
        double P = kFps * 60.0 / bpm;
        int bins = (int)std::ceil(P);
        hist.assign(bins, 0.0);
        for (const auto& [h, w] : hits) hist[(int)std::fmod((double)h, P) % bins] += w;
        for (int k = 0; k < bins; ++k) {
            double v = hist[k] + 0.5 * (hist[(k + 1) % bins] + hist[(k + bins - 1) % bins]);
            double ratio = v / (2.0 * total / bins);
            if (ratio > bestRatio) {
                bestRatio = ratio;
                bestBpm = bpm;
                bestPhase = k + 0.5;
            }
        }
    }
    double phaseSec = bestPhase / kFps;
    while (bestBpm < 72.0) bestBpm *= 2.0;
    while (bestBpm > 145.0) bestBpm *= 0.5;
    bpm_ = bestBpm;
    offset_ = std::fmod(phaseSec, 60.0 / bpm_);

    low_ = eLow;
    high_ = eHigh;
    loud_ = smooth(eAll, (int)(kFps * 1.5));
    for (auto& x : low_) x = std::sqrt(x);
    for (auto& x : high_) x = std::sqrt(x);
    normalise(low_);
    normalise(high_);
    normalise(loud_);

    std::vector<float> slow = smooth(loud_, (int)(kFps * 3.0));
    float qLow = percentile(slow, 0.3f), qHigh = percentile(slow, 0.65f);
    using Sec = music::Section;
    std::vector<uint8_t> raw(hops);
    for (int h = 0; h < hops; ++h)
        raw[h] = (uint8_t)(slow[h] >= qHigh ? Sec::Drop : slow[h] <= qLow ? Sec::Break : Sec::Build);
    const int minLen = (int)(kFps * 2.0);
    section_.assign(hops, (uint8_t)Sec::Intro);
    uint8_t cur = raw[0];
    for (int h = 0; h < hops; ++h) {
        if (raw[h] != cur) {
            int k = h;
            while (k < hops && k < h + minLen && raw[k] == raw[h]) ++k;
            if (k - h >= minLen || k == hops) cur = raw[h];
        }
        Sec sec = (Sec)cur;
        double t = h / kFps;
        if (t < 8.0 && sec != Sec::Drop) sec = Sec::Intro;
        if (sec == Sec::Drop && t > duration() * 0.6) sec = Sec::Drop2;
        section_[h] = (uint8_t)sec;
    }

    float kickThr = percentile(onLow, 0.93f);
    double lastKick = -1.0;
    for (int h = 1; h + 1 < hops; ++h)
        if (onLow[h] > kickThr && onLow[h] >= onLow[h - 1] && onLow[h] >= onLow[h + 1] && h / kFps - lastKick > 0.25) {
            lastKick = h / kFps;
            kicks_.push_back(lastKick);
        }

    double beatSec = 60.0 / bpm_;
    bool halfBeats = beatSec * 0.5 >= 0.22;
    struct Cand {
        double t, score;
        float lo, hi;
        int index;
    };
    std::vector<Cand> cand;
    std::vector<float> onSmooth = smooth(onset, (int)(kFps * 4.0));
    for (int j = 0;; ++j) {
        double t = offset_ + j * beatSec * 0.5;
        if (t > duration() - 1.0) break;
        if (t < 2.0 || (j % 2 && !halfBeats)) continue;
        int c = (int)std::lround(t * kFps);
        float s = 0, lo = 0, hi = 0;
        for (int k = std::max(0, c - 2); k <= std::min(hops - 1, c + 2); ++k) {
            s = std::max(s, onset[k]);
            lo = std::max(lo, onLow[k]);
            hi = std::max(hi, onHigh[k]);
        }
        if (s <= 0.0f || loud_[std::min(c, hops - 1)] < 0.05f) continue;
        double score = s / (onSmooth[std::min(c, hops - 1)] + 1e-3);
        if (j % 2) score *= 0.6;
        cand.push_back({t, score, lo, hi, j});
    }
    double slotsPerSec = (halfBeats ? 2.0 : 1.0) / beatSec;
    size_t target = std::min(cand.size(), (size_t)(duration() * std::min(2.0, 0.75 * slotsPerSec)));
    std::vector<Cand> pick = cand;
    std::nth_element(pick.begin(), pick.begin() + target, pick.end(), [](const Cand& a, const Cand& b) { return a.score > b.score; });
    pick.resize(target);
    std::sort(pick.begin(), pick.end(), [](const Cand& a, const Cand& b) { return a.t < b.t; });
    double pyroScore = 0.0;
    {
        std::vector<float> sc;
        for (auto& c : pick) sc.push_back((float)c.score);
        pyroScore = percentile(sc, 0.92f);
    }
    float meanLo = 1e-6f, meanHi = 1e-6f;
    for (auto& c : pick) {
        meanLo += c.lo / pick.size();
        meanHi += c.hi / pick.size();
    }
    notes_.clear();
    int side = 0;
    for (auto& c : pick) {
        int lane;
        if (c.score >= pyroScore && c.index % 2 == 0) lane = 2;
        else if (c.lo / meanLo > 1.4f * (c.hi / meanHi)) lane = 1;
        else {
            lane = side ? 3 : 0;
            side ^= 1;
        }
        if (!notes_.empty() && notes_.back().lane == lane && c.t - notes_.back().time < 0.3) lane = lane == 1 ? 0 : 1;
        notes_.push_back({c.t, lane});
    }
}

int Track::hopAt(double t) const {
    int h = (int)(t * kFps);
    return std::clamp(h, 0, (int)low_.size() - 1);
}

void Track::sample(long long n, float& l, float& r) const {
    if (n < 0 || n >= frames_) {
        l = r = 0.0f;
        return;
    }
    l = pcm_[2 * n];
    r = pcm_[2 * n + 1];
}

music::Section Track::section(double t) const {
    if (t >= duration()) return music::Section::Outro;
    return (music::Section)section_[hopAt(t)];
}

music::Levels Track::levels(double t) const {
    music::Levels lv;
    if (t < 0.0) return lv;
    int h = hopAt(t);
    double beatSec = 60.0 / bpm_;
    lv.beats = (float)std::max(0.0, (t - offset_) / beatSec);
    lv.bar = (int)(lv.beats / 4.0f);
    lv.section = section(t);
    auto it = std::upper_bound(kicks_.begin(), kicks_.end(), t);
    if (it != kicks_.begin()) {
        lv.kickAge = (float)(t - *(it - 1));
        lv.kick = std::exp(-6.0f * lv.kickAge) * (0.4f + 0.6f * low_[h]);
    }
    lv.bass = low_[h];
    lv.hat = high_[h];
    lv.snare = high_[h] * 0.7f;
    lv.lead = high_[h];
    lv.pad = loud_[h];
    return lv;
}
