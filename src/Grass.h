#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

class Shader;

class Grass {
public:
    bool init(int blades, float fieldSize);
    void draw(Shader& s, const glm::vec3& center, float wind) const;
    void release();

private:
    GLuint vao_ = 0, vboBlade_ = 0, vboInst_ = 0;
    int bladeVertexCount_ = 0, blades_ = 0;
    float fieldSize_ = 72.0f;
};
