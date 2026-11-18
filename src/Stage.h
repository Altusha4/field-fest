#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

class Shader;

class Stage {
public:
    static constexpr float kScreen[4] = {-8.2f, 1.9f, 8.2f, 8.0f};
    static constexpr int kSpotFixtures = 8;
    static float spotX(int i) { return -8.75f + 2.5f * i; }
    static constexpr float kPyroX[4] = {-7.0f, -2.5f, 2.5f, 7.0f};

    void init();
    void draw(Shader& s, const glm::vec3& accent, float showOn) const;
    void release();

private:
    GLuint vao_ = 0, vbo_ = 0;
    int count_ = 0;
};
