#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class GlowSprites {
public:
    struct Sprite {
        glm::vec4 posSize, color;
    };
    void init(int maxSprites);
    void draw(Shader& s, const std::vector<Sprite>& sprites) const;
    void release();

private:
    GLuint vao_ = 0, vbo_ = 0;
    int max_ = 0;
};
