#include "Camera.h"
#include "Location.h"
#include "MathUtil.h"
#include "Terrain.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
glm::vec3 dirFromAngles(float yaw, float pitch) {
    return glm::vec3(-std::sin(yaw) * std::cos(pitch), std::sin(pitch), -std::cos(yaw) * std::cos(pitch));
}
struct Preset {
    const char* name;
    glm::vec3 focus;
    float yaw, pitch, dist;
};
const Preset kPresetList[Camera::kPresets] = {
    {"behind the crowd", {0.0f, 4.0f, 0.0f}, 0.0f, 0.13f, 32.0f},
    {"from the stage", {0.0f, 2.0f, 18.0f}, mathx::kPi, 0.18f, 17.0f},
    {"aerial", {0.0f, 2.0f, 6.0f}, 0.6f, 0.5f, 50.0f},
    {"front row", {0.0f, 4.0f, -3.0f}, 0.25f, 0.05f, 14.0f},
    {"artists close-up", {0.0f, 2.4f, -1.6f}, 0.0f, 0.06f, 6.5f},
};
constexpr float kFollow = 3.0f;
}

const char* Camera::presetName(int i) { return kPresetList[i].name; }

void Camera::setPreset(int i, bool instant) {
    preset_ = mathx::wrap(i, kPresets);
    const Preset& p = kPresetList[preset_];
    goalYaw_ = yaw_ + std::remainder(p.yaw - yaw_, mathx::kTau);
    goalPitch_ = p.pitch;
    goalDist_ = p.dist;
    goalFocus_ = p.focus;
    mode = Mode::Orbit;
    if (instant) {
        yaw_ = goalYaw_;
        pitch_ = goalPitch_;
        dist_ = goalDist_;
        focus_ = goalFocus_;
    }
}

void Camera::rotate(float dx, float dy) {
    if (mode == Mode::Orbit) {
        goalYaw_ -= dx * 0.005f;
        goalPitch_ = std::clamp(goalPitch_ + dy * 0.004f, -0.05f, 1.3f);
    } else {
        freeYaw_ -= dx * 0.004f;
        freePitch_ = std::clamp(freePitch_ - dy * 0.004f, -1.5f, 1.5f);
    }
}

void Camera::zoom(float scrollY) {
    if (mode == Mode::Orbit) goalDist_ = std::clamp(goalDist_ * (1.0f - scrollY * 0.08f), 6.0f, 90.0f);
}

void Camera::toggleMode() {
    if (mode == Mode::Orbit) {
        glm::vec3 f = forward();
        freePos_ = eye_;
        freeYaw_ = std::atan2(-f.x, -f.z);
        freePitch_ = std::asin(std::clamp(f.y, -1.0f, 1.0f));
        mode = Mode::FreeFly;
    } else {
        mode = Mode::Orbit;
    }
}

void Camera::update(const glm::vec3& move, float speed, float dt, float time) {
    if (mode == Mode::Orbit) {
        yaw_ = mathx::approach(yaw_, goalYaw_, kFollow, dt);
        pitch_ = mathx::approach(pitch_, goalPitch_, kFollow, dt);
        dist_ = mathx::approach(dist_, goalDist_, kFollow, dt);
        focus_ += (goalFocus_ - focus_) * (1.0f - std::exp(-kFollow * dt));
        float drift = 0.06f * std::sin(time * 0.11f);
        float y = yaw_ + drift;
        eye_ = focus_ + glm::vec3(std::sin(y) * std::cos(pitch_), std::sin(pitch_), std::cos(y) * std::cos(pitch_)) * dist_;
        eye_.y = std::max(eye_.y, std::max(Terrain::height(eye_.x, eye_.z), location::kWaterLevel) + 0.5f);
        target_ = focus_;
    } else {
        glm::vec3 f = dirFromAngles(freeYaw_, freePitch_);
        glm::vec3 r = glm::normalize(glm::cross(f, glm::vec3(0, 1, 0)));
        freePos_ += (r * move.x + glm::vec3(0, 1, 0) * move.y + f * move.z) * speed * dt;
        freePos_.y = std::max(freePos_.y, std::max(Terrain::height(freePos_.x, freePos_.z), location::kWaterLevel) + 0.3f);
        eye_ = freePos_;
        target_ = freePos_ + f;
        focus_ = freePos_;
    }
}

glm::mat4 Camera::view() const { return glm::lookAt(eye_, target_, glm::vec3(0, 1, 0)); }
glm::mat4 Camera::projection(float aspect) const { return glm::perspective(fovY, aspect, nearZ, farZ); }
glm::vec3 Camera::forward() const { return glm::normalize(target_ - eye_); }
glm::vec3 Camera::right() const { return glm::normalize(glm::cross(forward(), glm::vec3(0, 1, 0))); }
glm::vec3 Camera::up() const { return glm::cross(right(), forward()); }
