#pragma once
#include <array>
#include <glm/glm.hpp>

class Shader;

struct FrameUniforms {
    static constexpr int kSpots = 8, kPoints = 10;

    glm::mat4 viewProj{1.0f};
    glm::vec3 camPos{0.0f}, camRight{1, 0, 0}, camUp{0, 1, 0}, camForward{0, 0, -1};
    glm::vec2 tanHalfFov{1.0f};
    glm::vec3 sunDir{0, 0.2f, -1}, sunColor{1.0f}, ambient{0.2f}, skyHorizon{1.0f}, skyZenith{0.2f};
    float night = 0.0f;
    float time = 0.0f;

    std::array<glm::vec3, kSpots> spotPos{}, spotDir{}, spotColor{};
    float spotCos = 0.99f;
    std::array<glm::vec3, kPoints> pointPos{}, pointColor{};

    float kickAge = 9.0f, kickPower = 0.0f, beats = 0.0f, hype = 0.5f;
    glm::vec4 levels{0.0f};

    void computeLighting(float azimuth, float altitude);
    void apply(Shader& s) const;
};
