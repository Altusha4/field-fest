#include "Grass.h"
#include "MathUtil.h"
#include "Shader.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

bool Grass::init(int maxBlades, float fieldSize) {
    fieldSize_ = fieldSize;
    blades_ = maxBlades;

    std::vector<float> blade;
    const int seg = 4;
    for (int i = 0; i < seg; ++i) {
        float y = (float)i / seg;
        blade.insert(blade.end(), {-1.0f, y, 1.0f, y});
    }
    blade.insert(blade.end(), {0.0f, 1.0f});
    bladeVertexCount_ = (int)blade.size() / 2;

    int side = (int)std::ceil(std::sqrt((double)maxBlades));
    float cell = fieldSize / side;
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    std::vector<std::pair<glm::vec4, glm::vec4>> blades;
    blades.reserve((size_t)maxBlades);
    for (int z = 0; z < side && (int)blades.size() < maxBlades; ++z)
        for (int x = 0; x < side && (int)blades.size() < maxBlades; ++x) {
            float px = (x + U(rng)) * cell - fieldSize * 0.5f;
            float pz = (z + U(rng)) * cell - fieldSize * 0.5f;
            float height = 0.45f + 0.75f * U(rng);
            float angle = U(rng) * mathx::kTau;
            float width = 0.035f * (0.6f + 0.8f * U(rng));
            blades.push_back({glm::vec4(px, pz, height, angle), glm::vec4(width, U(rng), U(rng), 0.0f)});
        }
    std::shuffle(blades.begin(), blades.end(), rng);
    std::vector<glm::vec4> inst;
    inst.reserve(blades.size() * 2);
    for (auto& b : blades) {
        inst.push_back(b.first);
        inst.push_back(b.second);
    }

    glGenBuffers(1, &vboBlade_);
    glBindBuffer(GL_ARRAY_BUFFER, vboBlade_);
    glBufferData(GL_ARRAY_BUFFER, blade.size() * sizeof(float), blade.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vboInst_);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
    glBufferData(GL_ARRAY_BUFFER, inst.size() * sizeof(glm::vec4), inst.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vboBlade_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4), nullptr);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec4), (void*)sizeof(glm::vec4));
    glVertexAttribDivisor(2, 1);
    glBindVertexArray(0);

    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "[grass] OpenGL error while creating buffers\n";
        return false;
    }
    std::cout << "[grass] " << maxBlades << " blades ready (" << (inst.size() * sizeof(glm::vec4)) / (1024 * 1024)
              << " MB instance data)\n";
    return true;
}

void Grass::draw(Shader& s, const glm::vec3& center, float wind) const {
    s.set("uCenter", glm::vec2(center.x, center.z));
    s.set("uFieldSize", fieldSize_);
    s.set("uWind", wind);
    glBindVertexArray(vao_);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, bladeVertexCount_, blades_);
}

void Grass::release() {
    glDeleteBuffers(1, &vboBlade_);
    glDeleteBuffers(1, &vboInst_);
    glDeleteVertexArrays(1, &vao_);
}
