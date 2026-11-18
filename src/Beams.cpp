#include "Beams.h"
#include "MathUtil.h"
#include "Shader.h"

#include <algorithm>
#include <cmath>

void Beams::init(int maxBeams) {
    max_ = maxBeams;
    std::vector<float> v;
    const int seg = 20;
    for (int i = 0; i < seg; ++i) {
        float a0 = mathx::kTau * i / seg, a1 = mathx::kTau * (i + 1) / seg;
        float c0 = std::cos(a0), s0 = std::sin(a0), c1 = std::cos(a1), s1 = std::sin(a1);
        v.insert(v.end(), {c0, s0, 0, c0, s0, 1, c1, s1, 1, c0, s0, 0, c1, s1, 1, c1, s1, 0});
    }
    vertexCount_ = (int)v.size() / 3;
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vboCone_);
    glGenBuffers(1, &vboInst_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vboCone_);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
    glBufferData(GL_ARRAY_BUFFER, maxBeams * sizeof(Instance), nullptr, GL_DYNAMIC_DRAW);
    for (int a = 0; a < 3; ++a) {
        glEnableVertexAttribArray(1 + a);
        glVertexAttribPointer(1 + a, 4, GL_FLOAT, GL_FALSE, sizeof(Instance), (void*)(a * sizeof(glm::vec4)));
        glVertexAttribDivisor(1 + a, 1);
    }
    glBindVertexArray(0);
}

void Beams::draw(Shader& s, const std::vector<Instance>& beams, float gain) const {
    int n = std::min((int)beams.size(), max_);
    if (n == 0) return;
    s.set("uBeamGain", gain);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, n * sizeof(Instance), beams.data());
    glBindVertexArray(vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount_, n);
}

void Beams::release() {
    glDeleteBuffers(1, &vboCone_);
    glDeleteBuffers(1, &vboInst_);
    glDeleteVertexArrays(1, &vao_);
}
