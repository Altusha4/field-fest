#pragma once
#include <algorithm>
#include <cmath>

namespace mathx {

constexpr float kPi = 3.14159265f;
constexpr float kTau = 6.2831853f;
constexpr double kTauD = 6.283185307179586;

inline int wrap(int i, int n) { return (i % n + n) % n; }

inline float smoothstep(float e0, float e1, float x) {
    float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float approach(float cur, float target, float rate, float dt) {
    return cur + (target - cur) * (1.0f - std::exp(-rate * dt));
}

}
