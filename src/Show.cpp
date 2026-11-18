#include "Show.h"
#include "FrameUniforms.h"
#include "Location.h"
#include "MathUtil.h"
#include "Stage.h"
#include "Terrain.h"

#include <algorithm>
#include <cmath>

static_assert(Show::kSpots == Stage::kSpotFixtures && Show::kSpots == FrameUniforms::kSpots,
              "one beam per spotlight fixture, one shader light per beam");

namespace {
using mathx::approach;
using mathx::kPi;
constexpr int kMaxParticles = 6000;
constexpr float kBeamLength = 38.0f;
constexpr float kBeamHalfAngle = 0.12f;

glm::vec3 hsv(float h, float s, float v) {
    h = h - std::floor(h);
    glm::vec3 p = glm::abs(glm::fract(glm::vec3(h) + glm::vec3(0.0f, 2.0f / 3.0f, 1.0f / 3.0f)) * 6.0f - 3.0f);
    return v * glm::mix(glm::vec3(1.0f), glm::clamp(p - 1.0f, 0.0f, 1.0f), s);
}
glm::vec3 hueRotate(const glm::vec3& c, float a) {
    const glm::vec3 k(0.57735f);
    float cs = std::cos(a), sn = std::sin(a);
    return glm::max(c * cs + glm::cross(k, c) * sn + k * glm::dot(k, c) * (1.0f - cs), glm::vec3(0.0f));
}

void palette(music::Section s, float beats, int i, glm::vec3& out) {
    using S = music::Section;
    bool odd = i % 2;
    switch (s) {
        case S::Intro: out = odd ? glm::vec3(1.0f, 0.88f, 0.7f) : glm::vec3(1.0f, 0.62f, 0.3f); break;
        case S::Build: out = odd ? glm::vec3(0.3f, 0.4f, 1.0f) : glm::vec3(0.95f, 0.25f, 0.8f); break;
        case S::Drop: out = odd ? glm::vec3(1.0f, 0.2f, 0.75f) : glm::vec3(0.2f, 0.9f, 1.0f); break;
        case S::Break: out = odd ? glm::vec3(0.15f, 0.5f, 1.0f) : glm::vec3(0.45f, 0.25f, 1.0f); break;
        case S::Build2: out = odd ? glm::vec3(1.0f) : glm::vec3(1.0f, 0.78f, 0.3f); break;
        case S::Drop2:
        case S::Outro: out = hsv(i / 8.0f + beats * 0.04f, 0.75f, 1.0f); break;
    }
    out = hueRotate(out, music::song(music::currentSong()).hueShift);
}
}

void Show::init(const std::vector<glm::vec3>& phones) {
    phonePos_ = phones;
    for (int i = 0; i < kSpots; ++i) {
        spotPos_[i] = glm::vec3(Stage::spotX(i), 9.25f, -1.1f);
        spotDir_[i] = glm::normalize(glm::vec3(0.0f, -1.0f, 0.6f));
    }
    fireflies_.resize(90);
    for (auto& f : fireflies_) {
        f.offset = glm::vec2(rnd() - 0.5f, rnd() - 0.5f) * 60.0f;
        f.phase = rnd() * mathx::kTau;
        f.speed = 0.4f + 0.6f * rnd();
    }
    reset();
}

void Show::reset() {
    particles_.clear();
    pending_.clear();
    pyro_.fill(0.0f);
    laneFlash_.fill(0.0f);
    flashColor_.fill(glm::vec3(0.0f));
    strobe_ = laser_ = missDim_ = 0.0f;
    wasLive_ = false;
    lastSection_ = music::Section::Intro;
}

void Show::trigger(int lane, float strength) {
    laneFlash_[lane] = strength;
    switch (lane) {
        case 0:
        case 3: break;
        case 1:
            laser_ = std::max(laser_, strength);
            strobe_ = 0.25f;
            break;
        case 2:
            pyro_.fill(strength);
            break;
    }
}

void Show::miss() { missDim_ = 1.0f; }

void Show::burst(const glm::vec3& at, const glm::vec3& color) {
    for (int i = 0; i < 150 && (int)particles_.size() < kMaxParticles; ++i) {
        float z = rnd() * 2.0f - 1.0f, a = rnd() * 2.0f * kPi, r = std::sqrt(1.0f - z * z);
        glm::vec3 d(r * std::cos(a), z, r * std::sin(a));
        float life = 1.6f + 0.9f * rnd();
        glm::vec3 c = rnd() < 0.15f ? glm::vec3(1.0f) : color;
        particles_.push_back({at, d * (7.0f + 3.0f * rnd()), c, life, life, 0.2f, 2.5f, 0.9f});
    }
    flashPos_[flashNext_] = at;
    flashColor_[flashNext_] = color * 6.0f;
    flashNext_ = (flashNext_ + 1) % 3;
}

void Show::firework() {
    glm::vec3 at(rnd() * 40.0f - 20.0f, 22.0f + rnd() * 10.0f, rnd() * 30.0f - 12.0f);
    pending_.push_back({0.0f, at, hsv(rnd(), 0.7f, 1.0f)});
}

void Show::celebrate() {
    for (int i = 0; i < 16; ++i) {
        glm::vec3 at(rnd() * 44.0f - 22.0f, 20.0f + rnd() * 14.0f, rnd() * 30.0f - 12.0f);
        pending_.push_back({i * 0.3f + rnd() * 0.2f, at, hsv(rnd(), 0.7f, 1.0f)});
    }
}

void Show::update(float dt, float animTime, double songTime, const music::Levels& lv, float hype, bool live) {
    using S = music::Section;
    time_ = animTime;
    S sec = lv.section;
    float t = animTime;

    float target = 0.0f;
    if (live) {
        switch (sec) {
            case S::Intro: target = 0.25f; break;
            case S::Build: target = 0.45f + 0.15f * ((lv.bar - 8) / 8.0f); break;
            case S::Drop:
            case S::Drop2: target = 1.0f; break;
            case S::Break: target = 0.15f; break;
            case S::Build2: target = 0.5f + 0.2f * ((lv.bar - 32) / 8.0f); break;
            case S::Outro: target = 0.45f; break;
        }
    }
    energy_ = approach(energy_, target, 2.0f, dt);
    showOn_ = approach(showOn_, live ? 1.0f : 0.0f, live ? 1.5f : 3.0f, dt);
    kick_ = live ? lv.kick : 0.0f;

    if (live && wasLive_ && sec != lastSection_ && (sec == S::Drop || sec == S::Drop2))
        for (int i = 0; i < 4; ++i) {
            glm::vec3 at(rnd() * 36.0f - 18.0f, 24.0f + rnd() * 8.0f, rnd() * 20.0f - 10.0f);
            pending_.push_back({i * 0.22f, at, hsv(rnd(), 0.7f, 1.0f)});
        }
    lastSection_ = sec;
    wasLive_ = live;

    float hypeK = 0.15f + 0.85f * hype;
    jump_ = approach(jump_, energy_ * hypeK * 0.4f, 4.0f, dt);
    float armsTarget = sec == S::Break && live ? 0.55f : energy_ * (0.25f + 0.75f * hype);
    arms_ = approach(arms_, live ? armsTarget : 0.0f, 3.0f, dt);
    sway_ = approach(sway_, live ? 1.0f - 0.6f * energy_ : 0.35f, 2.0f, dt);
    kickPower_ = energy_ * (0.4f + 0.6f * hype);
    bool phonesUp = live && (sec == S::Break || sec == S::Outro);
    phones_ = approach(phones_, phonesUp ? 1.0f : 0.0f, 1.5f, dt);

    glm::vec3 a;
    palette(live ? sec : S::Intro, lv.beats, 0, a);
    accent_ = a;
    float sectionProgress = (float)std::fmod(songTime, 8.0 * music::bar()) / (8.0f * (float)music::bar());
    for (int i = 0; i < kSpots; ++i) {
        glm::vec3 p = spotPos_[i];
        float sx = p.x, fi = (float)i;
        glm::vec3 want;
        bool sky = false;
        if (!live) {
            want = glm::vec3(sx * 0.6f, 0.0f, 7.0f + 2.0f * std::sin(0.3f * t + fi));
        } else {
            switch (sec) {
                case S::Intro:
                    want = glm::vec3(sx * 1.3f + 5.0f * std::sin(0.35f * t + fi), 0.0f, 15.0f + 7.0f * std::cos(0.27f * t + 1.3f * fi));
                    break;
                case S::Build:
                case S::Build2: {
                    float w = 0.8f + 2.0f * sectionProgress;
                    want = glm::vec3(sx * 1.5f + 9.0f * std::sin(w * t + fi * 0.9f), 0.0f, 16.0f + 8.0f * std::sin(0.6f * t + fi));
                    break;
                }
                case S::Drop:
                case S::Drop2: {
                    float ph = lv.beats * kPi * 0.5f;
                    want = glm::vec3(sx * 1.2f + 12.0f * std::sin(ph + fi * 0.785f), 0.0f, 18.0f + 10.0f * std::sin(ph * 0.5f + fi * 1.3f));
                    if (lv.bar % 2) want.x = -want.x;
                    break;
                }
                case S::Break:
                case S::Outro: sky = true; break;
            }
        }
        glm::vec3 dir = sky ? glm::normalize(glm::vec3(0.35f * std::sin(0.25f * t + fi * 0.8f), 1.0f,
                                                        0.45f + 0.25f * std::cos(0.2f * t + fi)))
                            : glm::normalize(want - p);
        spotDir_[i] = glm::normalize(glm::mix(spotDir_[i], dir, 1.0f - std::exp(-7.0f * dt)));

        glm::vec3 col;
        palette(live ? sec : S::Intro, lv.beats, i, col);
        float inten = live ? 4.5f * (0.3f + 0.7f * energy_) * (0.55f + 0.45f * kick_) * (0.45f + 0.55f * hype) : 0.5f;
        inten *= 1.0f - 0.6f * missDim_;
        col *= inten;
        float flash = i < 4 ? laneFlash_[0] : laneFlash_[3];
        col += glm::vec3(1.0f) * 3.0f * flash * flash;
        if (strobe_ > 0.0f && std::fmod(t * 16.0f, 1.0f) < 0.5f) col += glm::vec3(3.0f);
        spotColor_[i] = col;
    }

    beams_.clear();
    float endR = kBeamLength * std::tan(kBeamHalfAngle);
    for (int i = 0; i < kSpots; ++i)
        beams_.push_back({glm::vec4(spotPos_[i], kBeamLength), glm::vec4(spotDir_[i], endR), glm::vec4(spotColor_[i] * 0.22f, 0.16f)});
    float laserAmt = std::max(laser_, live && sec == S::Drop2 ? 0.35f * energy_ : 0.0f);
    if (laserAmt > 0.02f)
        for (int k = 0; k < 10; ++k) {
            float ang = (k / 9.0f - 0.5f) * 1.7f + 0.35f * std::sin(t * 2.0f + k * 0.3f);
            float elev = 0.1f + 0.07f * std::sin(t * 3.0f + k);
            glm::vec3 d = glm::normalize(glm::vec3(std::sin(ang), elev, std::cos(ang)));
            glm::vec3 c = (k % 3 == 0 ? glm::vec3(1.0f, 0.15f, 0.2f) : glm::vec3(0.15f, 1.0f, 0.3f)) * laserAmt * 2.5f;
            beams_.push_back({glm::vec4(0.0f, 2.4f, -3.6f, 70.0f), glm::vec4(d, 0.12f), glm::vec4(c, 0.03f)});
        }

    for (int k = 0; k < 4; ++k) {
        if (pyro_[k] > 0.35f)
            for (int i = 0; i < 7 && (int)particles_.size() < kMaxParticles; ++i) {
                glm::vec3 vel((rnd() - 0.5f) * 1.6f, 10.0f + 6.0f * rnd() * pyro_[k], (rnd() - 0.5f) * 1.6f);
                float life = 0.7f + 0.5f * rnd();
                particles_.push_back({glm::vec3(Stage::kPyroX[k], 1.5f, 0.6f), vel, glm::vec3(1.0f, 0.45f + 0.4f * rnd(), 0.12f), life,
                                      life, 0.22f, 9.0f, 0.6f});
            }
        pyro_[k] = std::max(0.0f, pyro_[k] - dt * 1.6f);
    }

    for (auto& b : pending_) b.delay -= dt;
    for (auto& b : pending_)
        if (b.delay <= 0.0f) burst(b.pos, b.color);
    pending_.erase(std::remove_if(pending_.begin(), pending_.end(), [](const PendingBurst& b) { return b.delay <= 0.0f; }),
                   pending_.end());
    for (auto& c : flashColor_) c *= std::exp(-dt * 4.0f);

    for (auto& p : particles_) {
        p.vel.y -= p.gravity * dt;
        p.vel *= 1.0f - p.drag * dt;
        p.pos += p.vel * dt;
        p.life -= dt;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& p) { return p.life <= 0.0f; }),
                     particles_.end());

    for (auto& f : laneFlash_) f = std::max(0.0f, f - dt * 3.0f);
    strobe_ = std::max(0.0f, strobe_ - dt);
    laser_ = std::max(0.0f, laser_ - dt * 1.2f);
    missDim_ = std::max(0.0f, missDim_ - dt * 3.0f);
}

void Show::fillUniforms(FrameUniforms& fu) const {
    for (int i = 0; i < kSpots; ++i) {
        fu.spotPos[i] = spotPos_[i];
        fu.spotDir[i] = spotDir_[i];
        fu.spotColor[i] = spotColor_[i];
    }
    fu.spotCos = std::cos(kBeamHalfAngle * 1.25f);
    fu.pointPos[0] = glm::vec3(0.0f, 5.0f, -7.0f);
    fu.pointColor[0] = accent_ * showOn_ * (1.0f + 2.0f * kick_);
    for (int k = 0; k < 4; ++k) {
        fu.pointPos[1 + k] = glm::vec3(Stage::kPyroX[k], 2.5f, 0.8f);
        fu.pointColor[1 + k] = glm::vec3(1.0f, 0.5f, 0.15f) * pyro_[k] * 8.0f;
    }
    for (int k = 0; k < 3; ++k) {
        fu.pointPos[5 + k] = flashPos_[k];
        fu.pointColor[5 + k] = flashColor_[k];
    }
}

void Show::fillSprites(std::vector<GlowSprites::Sprite>& out, const glm::vec3& focus, float night) const {
    out.clear();
    for (int i = 0; i < kSpots; ++i) {
        float m = std::max({spotColor_[i].r, spotColor_[i].g, spotColor_[i].b, 1e-3f});
        out.push_back({glm::vec4(spotPos_[i] + spotDir_[i] * 0.3f, 0.55f), glm::vec4(spotColor_[i] / m, std::min(1.0f, m * 0.5f))});
    }
    for (const auto& p : particles_) {
        float a = p.life / p.maxLife;
        out.push_back({glm::vec4(p.pos, p.size * (0.5f + 0.5f * a)), glm::vec4(p.color, a)});
    }
    if (phones_ > 0.01f)
        for (size_t i = 0; i < phonePos_.size(); ++i) {
            glm::vec3 p = phonePos_[i] + glm::vec3(0.12f * std::sin(time_ * 1.3f + i), Terrain::height(phonePos_[i].x, phonePos_[i].z), 0.0f);
            out.push_back({glm::vec4(p, 0.11f), glm::vec4(0.85f, 0.9f, 1.0f, phones_)});
        }
    if (location::cur().snow) {
        const float box = 50.0f, height = 22.0f;
        for (int i = 0; i < 4500; ++i) {
            float hx = std::fmod(i * 0.6180339f, 1.0f), hz = std::fmod(i * 0.7548776f, 1.0f), hy = std::fmod(i * 0.5698403f, 1.0f);
            float speed = 0.8f + 0.6f * std::fmod(i * 0.3819f, 1.0f);
            float x = focus.x + (hx - 0.5f) * box + 0.6f * std::sin(time_ * 0.7f + i);
            float z = focus.z + (hz - 0.5f) * box + 0.6f * std::cos(time_ * 0.6f + i * 1.3f);
            float y = focus.y - 4.0f + height - std::fmod(hy * height + time_ * speed, height);
            out.push_back({glm::vec4(x, y, z, 0.045f), glm::vec4(0.9f, 0.95f, 1.0f, 0.45f)});
        }
    }
    auto wrap = [](float v, float c) { return c + std::fmod(std::fmod(v - c, 60.0f) + 90.0f, 60.0f) - 30.0f; };
    if (!location::cur().snow)
    for (const auto& f : fireflies_) {
        float x = wrap(f.offset.x + std::sin(time_ * 0.3f * f.speed + f.phase) * 2.0f, focus.x);
        float z = wrap(f.offset.y + std::cos(time_ * 0.25f * f.speed + f.phase * 1.7f) * 2.0f, focus.z);
        float y = Terrain::height(x, z) + 0.3f + 1.4f * (0.5f + 0.5f * std::sin(time_ * 0.7f * f.speed + f.phase));
        out.push_back({glm::vec4(x, y, z, 0.07f), glm::vec4(0.75f, 1.0f, 0.35f, 0.3f + 0.9f * night)});
    }
}
