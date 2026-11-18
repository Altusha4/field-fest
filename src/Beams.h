#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class Beams {
public:
    struct Instance {
        glm::vec4 posLen, dirRadius, colorStart;
    };
    void init(int maxBeams);
    void draw(Shader& s, const std::vector<Instance>& beams, float gain) const;
    void release();

private:
    GLuint vao_ = 0, vboCone_ = 0, vboInst_ = 0;
    int vertexCount_ = 0, max_ = 0;
};
