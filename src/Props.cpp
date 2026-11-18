#include "Props.h"
#include "Crowd.h"
#include "Geometry.h"
#include "Location.h"
#include "MathUtil.h"
#include "Shader.h"
#include "Terrain.h"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace {
using glm::vec3;
using glm::vec4;

std::vector<float> leafyTree() {
    std::vector<float> v;
    const vec4 bark(0.3f, 0.21f, 0.13f, 0), leaf(0.17f, 0.36f, 0.1f, 0), leaf2(0.24f, 0.45f, 0.12f, 0);
    geo::addBox(v, {-0.18f, 0.0f, -0.18f}, {0.18f, 3.0f, 0.18f}, bark);
    geo::addBox(v, {-1.6f, 2.6f, -1.5f}, {1.5f, 4.6f, 1.6f}, leaf);
    geo::addBox(v, {-1.0f, 4.2f, -1.1f}, {1.1f, 5.6f, 1.0f}, leaf2);
    geo::addBox(v, {0.4f, 3.2f, -0.6f}, {2.0f, 4.4f, 1.0f}, leaf2);
    geo::addBox(v, {-2.0f, 3.0f, -0.4f}, {-0.6f, 4.0f, 1.2f}, leaf);
    return v;
}

std::vector<float> palm() {
    std::vector<float> v;
    const vec4 bark(0.48f, 0.35f, 0.22f, 0), bark2(0.38f, 0.27f, 0.17f, 0), frond(0.2f, 0.5f, 0.14f, 0),
        frond2(0.3f, 0.6f, 0.18f, 0), nut(0.3f, 0.2f, 0.1f, 0);
    vec3 top(0.0f);
    for (int i = 0; i < 9; ++i) {
        float y = i * 0.85f, x = 0.02f * i * i, w = 0.2f - 0.008f * i;
        geo::addBox(v, {x - w, y, -w}, {x + w, y + 0.86f, w}, i % 2 ? bark : bark2);
        top = vec3(x, y + 0.86f, 0.0f);
    }
    for (int k = 0; k < 8; ++k) {
        float a = k * 0.785f + 0.3f;
        glm::mat4 m = glm::translate(glm::mat4(1.0f), top);
        m = glm::rotate(m, a, vec3(0, 1, 0));
        for (int j = 0; j < 4; ++j) {
            m = glm::rotate(m, j == 0 ? -0.35f : 0.32f, vec3(0, 0, 1));
            float w = 0.42f - 0.08f * j;
            geo::addBox(v, {0.0f, -0.03f, -w}, {0.95f, 0.03f, w}, j % 2 ? frond : frond2, m);
            m = glm::translate(m, vec3(0.9f, 0.0f, 0.0f));
        }
    }
    for (int k = 0; k < 3; ++k) {
        float a = k * 2.1f;
        vec3 c = top + vec3(0.25f * std::cos(a), -0.25f, 0.25f * std::sin(a));
        geo::addBox(v, c - vec3(0.13f), c + vec3(0.13f), nut);
    }
    return v;
}

std::vector<float> pine() {
    std::vector<float> v;
    const vec4 bark(0.28f, 0.2f, 0.13f, 0), needles(0.07f, 0.2f, 0.11f, 0), snow(0.9f, 0.93f, 0.98f, 0);
    geo::addBox(v, {-0.2f, 0.0f, -0.2f}, {0.2f, 1.6f, 0.2f}, bark);
    const float tiers[5][3] = {{3.4f, 1.2f, 1.2f}, {2.7f, 2.2f, 1.1f}, {2.0f, 3.2f, 1.0f}, {1.3f, 4.1f, 0.9f}, {0.6f, 4.9f, 0.8f}};
    for (int i = 0; i < 5; ++i) {
        float w = tiers[i][0] * 0.5f, y = tiers[i][1], h = tiers[i][2];
        glm::mat4 m = glm::rotate(glm::mat4(1.0f), i * 0.785f, vec3(0, 1, 0));
        geo::addBox(v, {-w, y, -w}, {w, y + h * 0.75f, w}, needles, m);
        geo::addBox(v, {-w * 0.8f, y + h * 0.75f, -w * 0.8f}, {w * 0.8f, y + h * 0.9f, w * 0.8f}, snow, m);
    }
    return v;
}

std::vector<vec4> place(int count, int location, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    std::vector<vec4> out;
    int prev = location::current();
    location::set(location);
    for (int tries = 0; (int)out.size() < count && tries < count * 50; ++tries) {
        float a = U(rng) * mathx::kTau, r = 22.0f + 55.0f * std::sqrt(U(rng));
        float x = std::cos(a) * r, z = 6.0f + std::sin(a) * r;
        if (z > 0.0f && z < 45.0f && std::fabs(x) < Crowd::fanHalfWidth(z) + 4.0f) continue;
        if (std::fabs(x) < 18.0f && z < 8.0f && z > -14.0f) continue;
        float h = Terrain::height(x, z);
        if (location == location::Beach && (h < location::kWaterLevel + 0.4f || h > 2.5f)) continue;
        out.emplace_back(x, z, 1.0f + 0.6f * U(rng), U(rng) * mathx::kTau);
    }
    location::set(prev);
    return out;
}
}

void Props::upload(Kind& k, const std::vector<float>& mesh, const std::vector<vec4>& inst) {
    k.vertexCount = (int)mesh.size() / geo::kFloatsPerVertex;
    k.count = (int)inst.size();
    glGenVertexArrays(1, &k.vao);
    glGenBuffers(1, &k.vboMesh);
    glGenBuffers(1, &k.vboInst);
    glBindVertexArray(k.vao);
    glBindBuffer(GL_ARRAY_BUFFER, k.vboMesh);
    glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(float), mesh.data(), GL_STATIC_DRAW);
    geo::setMeshAttribs();
    glBindBuffer(GL_ARRAY_BUFFER, k.vboInst);
    glBufferData(GL_ARRAY_BUFFER, inst.size() * sizeof(vec4), inst.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), nullptr);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void Props::init() {
    upload(kinds_[location::Meadow], leafyTree(), place(80, location::Meadow, 11));
    upload(kinds_[location::Beach], palm(), place(80, location::Beach, 12));
    upload(kinds_[location::Winter], pine(), place(180, location::Winter, 13));
}

void Props::draw(Shader&, int location) const {
    const Kind& k = kinds_[location];
    glBindVertexArray(k.vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, k.vertexCount, k.count);
}

void Props::release() {
    for (Kind& k : kinds_) {
        glDeleteBuffers(1, &k.vboMesh);
        glDeleteBuffers(1, &k.vboInst);
        glDeleteVertexArrays(1, &k.vao);
    }
}
