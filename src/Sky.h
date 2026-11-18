#pragma once
#include <glad/gl.h>

class Shader;

class Sky {
public:
    void init();
    void draw(Shader& s) const;
    void release();

private:
    GLuint vao_ = 0;
};
