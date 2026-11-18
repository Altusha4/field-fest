#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

class Shader;

class Performers {
public:
    enum class Cast { None, Duo, Singer };
    void init();
    void update(float time, float beats, float energy, Cast cast);
    void draw(Shader& s) const;
    void release();
    std::array<glm::vec3, 2> spotTargets() const { return spot_; }
    int spotCount() const { return spotCount_; }

private:
    enum Joint { Body, Head, LUpper, LFore, RUpper, RFore, LLeg, RLeg, JointCount };
    struct Part {
        glm::vec3 mn, mx, color;
        Joint joint;
    };
    struct Pose {
        glm::vec3 pos{0.0f};
        float yaw = 0, bounce = 0, nod = 0, turn = 0, legSwing = 0;
        glm::vec2 lUpper{0}, lFore{0}, rUpper{0}, rFore{0};
    };
    using Character = std::vector<Part>;
    void build(const Character& c, const Pose& p);

    std::vector<Character> chars_;
    std::vector<float> verts_;
    GLuint vao_ = 0, vbo_ = 0;
    int count_ = 0, capacity_ = 0;
    std::array<glm::vec3, 2> spot_{};
    int spotCount_ = 0;
};
