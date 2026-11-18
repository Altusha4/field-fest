#pragma once
#include "Beams.h"
#include "GlowSprites.h"
#include "Music.h"

#include <array>
#include <glm/glm.hpp>
#include <random>
#include <vector>

struct FrameUniforms;

class Show {
public:
    static constexpr int kSpots = 8;

    void init(const std::vector<glm::vec3>& phones);
    void reset();
    void trigger(int lane, float strength);
    void miss();
    void firework();
    void celebrate();
    void update(float dt, float animTime, double songTime, const music::Levels& lv, float hype, bool live);

    void fillUniforms(FrameUniforms& fu) const;
    const std::vector<Beams::Instance>& beams() const { return beams_; }
    void fillSprites(std::vector<GlowSprites::Sprite>& out, const glm::vec3& camFocus, float night) const;

    glm::vec3 accent() const { return accent_; }
    float showOn() const { return showOn_; }
    float jump() const { return jump_; }
    float arms() const { return arms_; }
    float sway() const { return sway_; }
    float kickPower() const { return kickPower_; }
    float laneFlash(int lane) const { return laneFlash_[lane]; }

private:
    struct Particle {
        glm::vec3 pos, vel, color;
        float life, maxLife, size, gravity, drag;
    };
    struct PendingBurst {
        float delay;
        glm::vec3 pos, color;
    };
    void burst(const glm::vec3& at, const glm::vec3& color);
    float rnd() { return dist_(rng_); }

    std::array<glm::vec3, kSpots> spotPos_{}, spotDir_{}, spotColor_{};
    std::array<float, 4> pyro_{}, laneFlash_{};
    std::array<glm::vec3, 3> flashPos_{}, flashColor_{};
    int flashNext_ = 0;
    float strobe_ = 0, laser_ = 0, missDim_ = 0, energy_ = 0, showOn_ = 0, phones_ = 0;
    float jump_ = 0, arms_ = 0, sway_ = 0.3f, kickPower_ = 0, kick_ = 0, time_ = 0;
    glm::vec3 accent_{1.0f, 0.75f, 0.45f};
    music::Section lastSection_ = music::Section::Intro;
    bool wasLive_ = false;

    std::vector<Particle> particles_;
    std::vector<PendingBurst> pending_;
    std::vector<Beams::Instance> beams_;
    std::vector<glm::vec3> phonePos_;
    struct Firefly {
        glm::vec2 offset;
        float phase, speed;
    };
    std::vector<Firefly> fireflies_;
    std::mt19937 rng_{42};
    std::uniform_real_distribution<float> dist_{0.0f, 1.0f};
};
