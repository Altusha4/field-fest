#include "Sky.h"
#include "Shader.h"

void Sky::init() { glGenVertexArrays(1, &vao_); }

void Sky::draw(Shader&) const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Sky::release() { glDeleteVertexArrays(1, &vao_); }
