#pragma once
#include <glm/glm.hpp>

class Camera {
public:
    enum class Mode { Orbit, FreeFly };
    static constexpr int kPresets = 5;

    Mode  mode = Mode::Orbit;
    float fovY = glm::radians(55.0f);
    float nearZ = 0.1f, farZ = 400.0f;

    void rotate(float dx, float dy);
    void zoom(float scrollY);
    void toggleMode();
    void setPreset(int i, bool instant = false);
    int preset() const { return preset_; }
    static const char* presetName(int i);
    void update(const glm::vec3& freeMove, float speed, float dt, float time);

    glm::mat4 view() const;
    glm::mat4 projection(float aspect) const;
    glm::vec3 eye() const { return eye_; }
    glm::vec3 focus() const { return focus_; }
    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

private:
    float yaw_ = 0, pitch_ = 0.12f, dist_ = 40;
    float goalYaw_ = 0, goalPitch_ = 0.12f, goalDist_ = 40;
    glm::vec3 focus_{0, 3.5f, 2}, goalFocus_{0, 3.5f, 2};
    int preset_ = 0;
    glm::vec3 freePos_{0.0f, 3.0f, 8.0f};
    float freeYaw_ = 0.0f, freePitch_ = -0.1f;
    glm::vec3 eye_{0.0f}, target_{0.0f, 0.0f, -1.0f};
};
