#include "GlowSprites.h"
#include "Shader.h"

#include <algorithm>

void GlowSprites::init(int maxSprites) {
    max_ = maxSprites;
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, maxSprites * sizeof(Sprite), nullptr, GL_DYNAMIC_DRAW);
    for (int a = 0; a < 2; ++a) {
        glEnableVertexAttribArray(a);
        glVertexAttribPointer(a, 4, GL_FLOAT, GL_FALSE, sizeof(Sprite), (void*)(a * sizeof(glm::vec4)));
        glVertexAttribDivisor(a, 1);
    }
    glBindVertexArray(0);
}

void GlowSprites::draw(Shader&, const std::vector<Sprite>& sprites) const {
    int n = std::min((int)sprites.size(), max_);
    if (n == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, n * sizeof(Sprite), sprites.data());
    glBindVertexArray(vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, n);
}

void GlowSprites::release() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}
