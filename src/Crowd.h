#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class Crowd {
public:
    static constexpr int kMax = 50000;
    static constexpr int kDefault = 3000;

    void init();
    int draw(Shader& s, int count, bool instanced, float jump, float arms, float sway) const;
    void release();
    int maxCount() const { return (int)inst_.size() / 2; }
    static float fanHalfWidth(float z) { return 10.0f + 0.7f * (z - 3.5f); }
    const std::vector<glm::vec3>& phones() const { return phones_; }

private:
    GLuint vao_ = 0, vaoNaive_ = 0, vboMesh_ = 0, vboInst_ = 0;
    int vertexCount_ = 0;
    std::vector<glm::vec4> inst_;
    std::vector<glm::vec3> phones_;
};
