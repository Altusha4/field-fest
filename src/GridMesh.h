#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>

class Shader;

class GridMesh {
public:
    void init(int cells, float size);
    void draw(Shader& s, const glm::vec3& center) const;
    void release();

private:
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    int cells_ = 0, indexCount_ = 0;
    float size_ = 1.0f;
};
