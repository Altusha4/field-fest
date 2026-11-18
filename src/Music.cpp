#include "Music.h"
#include "MathUtil.h"
#include "Track.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>

namespace music {
namespace {

constexpr double kTau = mathx::kTauD;

std::vector<Song> gSongs = {
    {"Sunset Drive", "house", "NORMAL", 128.0,
     {45, 41, 48, 43}, {{57, 60, 64}, {53, 57, 60}, {55, 60, 64}, {55, 59, 62}},
     1, 160.0f, 0.85f, 20.0f, 185.0f, 0.0f, 0, 5, false, false, 0.05f, 0.0f},
    {"Lazy Fields", "lo-fi chill", "EASY", 96.0,
     {41, 40, 38, 36}, {{57, 60, 64}, {55, 59, 62}, {53, 57, 60}, {52, 55, 59}},
     0, 110.0f, 0.75f, 45.0f, 330.0f, 0.1f, 2, 2, true, false, 0.065f, -0.9f},
    {"Neon Rush", "hard dance", "HARD", 150.0,
     {38, 46, 41, 48}, {{57, 62, 65}, {58, 62, 65}, {57, 60, 65}, {55, 60, 64}},
     2, 200.0f, 0.95f, 16.0f, 200.0f, 0.0f, 1, 7, false, true, 0.04f, 1.2f},
};
std::vector<std::unique_ptr<Track>> gTracks;
std::atomic<int> gSong{0};

const int kArpPattern[8] = {0, 1, 2, 3, 2, 1, 2, 3};

const Song& S() { return gSongs[gSong.load()]; }
const Track* T() { return S().track >= 0 ? gTracks[S().track].get() : nullptr; }

double mtof(int midi) { return 440.0 * std::pow(2.0, (midi - 69) / 12.0); }

float noise(long long n) {
    uint32_t h = (uint32_t)n * 747796405u + 2891336453u;
    h = ((h >> ((h >> 28u) + 4u)) ^ h) * 277803737u;
    h = (h >> 22u) ^ h;
    return (float)(h / 2147483648.0 - 1.0);
}

struct Mix {
    bool kick = false, clap = false, roll = false, hat = false, hat16 = false, bass = false;
    float arp = 0.0f, pad = 0.0f;
};

Section sectionOfBar(int bar) {
    if (bar < 8) return Section::Intro;
    if (bar < 16) return Section::Build;
    if (bar < 24) return Section::Drop;
    if (bar < 32) return Section::Break;
    if (bar < 40) return Section::Build2;
    if (bar < 48) return Section::Drop2;
    return Section::Outro;
}

Mix mixOfBar(int bar) {
    Mix m;
    bool busy = S().busyHats;
    switch (sectionOfBar(bar)) {
        case Section::Intro:
            m.kick = bar >= 4;
            m.hat = bar >= 6;
            m.pad = 1.0f;
            break;
        case Section::Build:
            m.kick = true;
            m.hat = !busy;
            m.hat16 = busy;
            m.clap = m.bass = bar >= 12;
            m.roll = bar >= 14;
            m.pad = 0.7f;
            break;
        case Section::Drop:
        case Section::Drop2:
            m.kick = m.clap = m.hat16 = m.bass = true;
            m.arp = 1.0f;
            m.pad = 0.5f;
            break;
        case Section::Break:
            m.pad = 1.0f;
            m.arp = 0.55f;
            break;
        case Section::Build2:
            m.kick = m.clap = true;
            m.hat = !busy;
            m.hat16 = busy;
            m.bass = bar >= 34;
            m.roll = bar >= 38;
            m.arp = 0.45f;
            m.pad = 0.7f;
            break;
        case Section::Outro: break;
    }
    return m;
}

float kickWave(double t) {
    if (t < 0.0 || t > 0.45) return 0.0f;
    double top = S().kickTop;
    double phase = kTau * (48.0 * t + (top - 48.0) * (1.0 - std::exp(-t * 28.0)) / 28.0);
    return (float)(std::sin(phase) * std::exp(-t * 6.0) + 0.25 * std::exp(-t * 300.0));
}
float kickEnv(double t) { return t < 0.0 ? 0.0f : (float)std::exp(-t * 6.0); }

float clapWave(double t, long long n) {
    if (t < 0.0 || t > 0.4) return 0.0f;
    double d = S().clapDecay;
    return (float)(noise(n) * std::exp(-t * d) * 0.5 + std::sin(kTau * S().clapTone * t) * std::exp(-t * (d + 10.0)) * 0.25);
}

float hatWave(double t, long long n, double decay) {
    if (t < 0.0 || t > 0.3) return 0.0f;
    float hp = noise(n) - noise(n - 1);
    return (float)(hp * 0.5 * std::exp(-t * decay));
}

float bassWave(double t, double f, double len, double decay) {
    if (t < 0.0 || t > len) return 0.0f;
    double v = 0.0;
    int harm = S().bassHarmonics;
    for (int k = 1; k <= harm; ++k) v += std::sin(kTau * f * k * t) / k * std::exp(-t * (decay + 7.0 * (k - 1)));
    double attack = std::min(1.0, t / 0.004), release = std::min(1.0, (len - t) / 0.03);
    return (float)(v * attack * release);
}

float leadWave(double t, double f) {
    if (t < 0.0) return 0.0f;
    if (S().softKeys) {
        if (t > 0.9) return 0.0f;
        double s = std::sin(kTau * f * t) + 0.25 * std::sin(kTau * 2.0 * f * t) * std::exp(-t * 6.0);
        return (float)(s * std::exp(-t * 4.0) * std::min(1.0, t / 0.006) * 0.8);
    }
    if (t > 0.25) return 0.0f;
    double s = std::sin(kTau * f * t);
    return (float)((std::tanh(3.0 * s) * 0.6 + 0.4 * std::sin(kTau * 2.0 * f * t)) * std::exp(-t * 11.0) *
                   std::min(1.0, t / 0.003));
}

void padWave(double tAbs, int chord, float gain, float& l, float& r) {
    for (int i = 0; i < 3; ++i) {
        double f = mtof(S().chord[chord][i]);
        double a = kTau * f * tAbs, b = kTau * f * 1.004 * tAbs;
        l += gain * (float)(std::sin(a) + 0.3 * std::sin(2.0 * a));
        r += gain * (float)(std::sin(b) + 0.3 * std::sin(2.0 * b));
    }
}

struct GridPos {
    double beats;
    int beat;
    double inBeat;
    int bar, beatInBar;
};
GridPos gridAt(double t) {
    GridPos g;
    g.beats = t / beat();
    g.beat = (int)g.beats;
    g.inBeat = (g.beats - g.beat) * beat();
    g.bar = g.beat / 4;
    g.beatInBar = g.beat % 4;
    return g;
}

double offBeat(double inBeat) { return inBeat - beat() * (0.5 + S().swing); }

bool bassNote(double inBeat, int beatInBar, int& octave, double& bt, double& len, double& decay) {
    double b = beat();
    octave = 0;
    switch (S().bassMode) {
        case 0:
            bt = offBeat(inBeat);
            len = 0.24;
            decay = 3.0;
            return bt >= 0.0;
        case 1: {
            int step = (int)(inBeat / (b * 0.25));
            if (step == 0) return false;
            bt = inBeat - step * b * 0.25;
            len = b * 0.25 * 0.9;
            decay = 4.0;
            octave = step == 2 ? 12 : 0;
            return true;
        }
        default:
            bt = inBeat + (beatInBar % 2) * b;
            len = b * 1.9;
            decay = 1.5;
            return true;
    }
}

}

int loadTracks(const std::string& folder) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path dir = fs::u8path(folder);
    if (!fs::is_directory(dir, ec)) return 0;
    std::vector<fs::path> files;
    for (const auto& e : fs::directory_iterator(dir, ec)) {
        std::string ext = e.path().extension().u8string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return (char)std::tolower(ch); });
        if (ext == ".mp3" || ext == ".wav" || ext == ".flac") files.push_back(e.path());
    }
    std::sort(files.begin(), files.end());
    int added = 0;
    for (const auto& f : files) {
        auto t = std::make_unique<Track>();
        if (!t->load(f.u8string())) continue;
        Song s = gSongs[0];
        s.title = t->title().c_str();
        s.style = "your track";
        s.difficulty = "AUTO";
        s.bpm = t->bpm();
        s.hueShift = 2.1f + 0.9f * added;
        s.track = (int)gTracks.size();
        gTracks.push_back(std::move(t));
        gSongs.push_back(s);
        ++added;
    }
    return added;
}

int songCount() { return (int)gSongs.size(); }
const Song& song(int i) { return gSongs[mathx::wrap(i, songCount())]; }
void setSong(int i) { gSong = mathx::wrap(i, songCount()); }
int currentSong() { return gSong.load(); }

double beat() { return 60.0 / S().bpm; }
double bar() { return 4.0 * beat(); }
double songEnd() { return T() ? T()->duration() : kBars * bar(); }
double length() { return T() ? T()->duration() + 0.5 : songEnd() + 3.0; }
double durationOf(int i) {
    const Song& s = song(i);
    return s.track >= 0 ? gTracks[s.track]->duration() : kBars * 4 * 60.0 / s.bpm;
}

Section sectionAt(double t) {
    if (T()) return T()->section(t);
    return t < 0.0 ? Section::Intro : sectionOfBar((int)(t / bar()));
}

const char* sectionName(Section s) {
    switch (s) {
        case Section::Intro: return "Intro";
        case Section::Build: return "Build-up";
        case Section::Drop: return "DROP";
        case Section::Break: return "Breakdown";
        case Section::Build2: return "Build-up 2";
        case Section::Drop2: return "FINAL DROP";
        case Section::Outro: return "Outro";
    }
    return "";
}

Levels levels(double t) {
    if (T()) return T()->levels(t);
    Levels lv;
    if (t < 0.0) return lv;
    const double B = beat();
    const GridPos g = gridAt(t);
    const double inBeat = g.inBeat;
    const int barI = g.bar, beatInBar = g.beatInBar;
    lv.beats = (float)g.beats;
    lv.bar = barI;
    lv.section = sectionOfBar(barI);
    if (barI >= kBars) {
        double tf = t - songEnd();
        lv.kick = kickEnv(tf);
        lv.kickAge = (float)tf;
        lv.pad = (float)std::exp(-tf * 1.2);
        return lv;
    }
    Mix m = mixOfBar(barI);
    if (m.kick) {
        lv.kick = kickEnv(inBeat);
        lv.kickAge = (float)inBeat;
    }
    if (m.clap && (beatInBar == 1 || beatInBar == 3)) lv.snare = (float)std::exp(-inBeat * 12.0);
    if (m.roll) lv.snare = std::max(lv.snare, (float)(0.5 + 0.5 * ((barI % 2) * 4 + beatInBar) / 8.0));
    double off = offBeat(inBeat);
    if (m.hat || m.hat16) lv.hat = off >= 0.0 ? (float)std::exp(-off * 20.0) : 0.0f;
    int oct;
    double bt, len, decay;
    if (m.bass && bassNote(inBeat, beatInBar, oct, bt, len, decay)) lv.bass = (float)std::exp(-bt * 3.0 * decay);
    double step = S().softKeys ? B * 0.5 : B * 0.25;
    lv.lead = m.arp * (float)std::exp(-std::fmod(inBeat, step) * (S().softKeys ? 4.0 : 11.0));
    lv.pad = m.pad;
    return lv;
}

void sample(double t, long long n, float& left, float& right) {
    left = right = 0.0f;
    if (const Track* tr = T()) {
        tr->sample(n, left, right);
        return;
    }
    if (t < 0.0 || t > length()) return;
    const Song& song = S();
    const double B = beat(), BAR = bar();
    float l = 0.0f, r = 0.0f;
    auto both = [&](float v) {
        l += v;
        r += v;
    };

    if (t >= songEnd()) {
        double tf = t - songEnd();
        float k = kickWave(tf) * song.kickGain + hatWave(tf, n, 2.5) * 0.6f;
        float pl = 0, pr = 0;
        padWave(t, 0, song.padGain * (float)std::exp(-tf * 1.2), pl, pr);
        l = k + pl;
        r = k + pr;
    } else {
        const GridPos g = gridAt(t);
        const double inBeat = g.inBeat;
        const int beatI = g.beat, barI = g.bar, beatInBar = g.beatInBar, chord = barI % 4;
        double tBar = t - barI * BAR;
        Mix m = mixOfBar(barI);

        float duck = 1.0f;
        if (m.kick) {
            both(kickWave(inBeat) * song.kickGain);
            duck = 1.0f - 0.65f * (float)std::exp(-inBeat * 9.0);
        }
        if (m.clap && (beatInBar == 1 || beatInBar == 3)) {
            both(clapWave(inBeat, n) * 0.8f);
        }
        if (m.roll) {
            double step = (barI % 2 == 0) ? B * 0.5 : B * 0.25;
            double rt = std::fmod(tBar, step);
            float gain = 0.25f + 0.5f * (float)(((barI % 2) * BAR + tBar) / (2.0 * BAR));
            both(clapWave(rt, n) * gain);
        }
        if (m.hat) {
            float h = hatWave(offBeat(inBeat), n, 22.0) * 0.35f;
            l += h * 0.8f;
            r += h;
        }
        if (m.hat16) {
            double s16 = B * 0.25;
            double ht = std::fmod(inBeat, s16);
            int idx = (int)(inBeat / s16);
            float h = hatWave(ht, n, idx == 2 ? 18.0 : 55.0) * (idx == 2 ? 0.4f : 0.2f);
            l += h * 0.8f;
            r += h;
        }
        int oct;
        double bt, len, decay;
        if (m.bass && bassNote(inBeat, beatInBar, oct, bt, len, decay)) {
            both(bassWave(bt, mtof(song.bassRoot[chord] + oct), len, decay) * 0.55f * duck);
        }
        if (m.arp > 0.0f) {
            double step = song.softKeys ? B * 0.5 : B * 0.25;
            int perBeat = song.softKeys ? 2 : 4;
            for (int back = 0; back < 2; ++back) {
                int s = beatI * perBeat + (int)(inBeat / step) - back;
                if (s < 0) continue;
                double nt = t - s * step;
                int c = (s / (perBeat * 4)) % 4;
                int idx = kArpPattern[s % 8];
                int midi = idx < 3 ? song.chord[c][idx] + 12 : song.chord[c][0] + 24;
                float a = leadWave(nt, mtof(midi)) * 0.16f * m.arp;
                float pan = (s % 2) ? 0.3f : 0.7f;
                l += a * (1.0f - pan) * 2.0f;
                r += a * pan * 2.0f;
            }
        }
        if (m.pad > 0.0f) {
            float fadeIn = (float)std::min(1.0, tBar / 0.25);
            float pl = 0, pr = 0;
            padWave(t, chord, song.padGain * m.pad * fadeIn * duck, pl, pr);
            if (fadeIn < 1.0f && barI > 0) {
                Mix pm = mixOfBar(barI - 1);
                padWave(t, (barI - 1) % 4, song.padGain * pm.pad * (1.0f - fadeIn) * duck, pl, pr);
            }
            l += pl;
            r += pr;
        }
    }
    left = std::tanh(l * 0.9f);
    right = std::tanh(r * 0.9f);
}

std::vector<Note> chart() {
    if (T()) return T()->notes();
    const Song& song = S();
    const int level = song.level;
    std::vector<Note> notes;
    auto add = [&](int barI, double b, int lane) {
        double whole = std::floor(b), frac = b - whole;
        bool drop = sectionOfBar(barI) == Section::Drop || sectionOfBar(barI) == Section::Drop2;
        if (level == 0 && (frac > 0.0 || (!drop && whole != 0.0 && whole != 2.0))) return;
        if (frac > 0.0) frac += song.swing;
        notes.push_back({(barI * 4 + whole + frac) * beat(), lane});
    };
    for (int barI = 2; barI < kBars; ++barI) {
        bool odd = barI % 2;
        switch (sectionOfBar(barI)) {
            case Section::Intro:
                add(barI, 0, odd ? 3 : 0);
                if (barI >= 4) add(barI, 2, odd ? 0 : 3);
                break;
            case Section::Build:
                if (barI < 12) {
                    for (int b = 0; b < 4; ++b) add(barI, b, b % 2 ? 3 : 0);
                } else if (barI < 15) {
                    const int p[4] = {0, 1, 3, 1};
                    for (int b = 0; b < 4; ++b) add(barI, b, p[b]);
                    if (level >= 2) {
                        add(barI, 1.5, 2);
                        add(barI, 3.5, 2);
                    }
                } else {
                    const int p[4] = {0, 3, 0, 3};
                    for (int b = 0; b < 4; ++b) add(barI, b, p[b]);
                    add(barI, 3.5, 1);
                }
                break;
            case Section::Drop:
            case Section::Drop2: {
                bool second = sectionOfBar(barI) == Section::Drop2;
                add(barI, 0, odd ? 1 : 2);
                const int a[3] = {0, 3, 1}, b[3] = {3, 0, 1};
                const int* p = odd ? b : a;
                for (int i = 0; i < 3; ++i) add(barI, 1 + i, p[i]);
                if (second || level >= 2) {
                    add(barI, 1.5, odd ? 0 : 3);
                    add(barI, 3.5, odd ? 3 : 0);
                } else if (odd) {
                    add(barI, 3.5, 3);
                }
                if (second && level >= 2) add(barI, 2.5, 2);
                break;
            }
            case Section::Break:
                add(barI, 0, 0);
                add(barI, 2, 3);
                if (barI >= 28) add(barI, 3, 1);
                break;
            case Section::Build2:
                if (barI < 36) {
                    for (int b = 0; b < 4; ++b) add(barI, b, b % 2 ? 3 : 0);
                } else if (barI < 38) {
                    const int p[4] = {0, 1, 3, 1};
                    for (int b = 0; b < 4; ++b) add(barI, b, p[b]);
                } else {
                    const double beats[6] = {0, 1, 2, 2.5, 3, 3.5};
                    for (int i = 0; i < 6; ++i) add(barI, beats[i], i % 2 ? 3 : 0);
                }
                break;
            case Section::Outro: break;
        }
    }
    add(kBars, 0, 2);
    std::sort(notes.begin(), notes.end(), [](const Note& x, const Note& y) { return x.time < y.time; });
    return notes;
}

}
