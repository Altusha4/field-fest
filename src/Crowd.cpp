#include "Crowd.h"
#include "Geometry.h"
#include "Shader.h"
#include "Terrain.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

void Crowd::init() {
    using glm::vec3;
    using glm::vec4;
    std::vector<float> m;
    geo::addBox(m, vec3(-0.17f, 0.0f, -0.08f), vec3(-0.03f, 0.86f, 0.08f), vec4(0, 2, 0, 0));
    geo::addBox(m, vec3(0.03f, 0.0f, -0.08f), vec3(0.17f, 0.86f, 0.08f), vec4(0, 2, 0, 0));
    geo::addBox(m, vec3(-0.22f, 0.85f, -0.12f), vec3(0.22f, 1.45f, 0.12f), vec4(0, 1, 0, 0));
    geo::addBox(m, vec3(-0.11f, 1.48f, -0.11f), vec3(0.11f, 1.74f, 0.11f), vec4(0, 0, 0, 0));
    geo::addBox(m, vec3(-0.115f, 1.66f, -0.115f), vec3(0.115f, 1.77f, 0.115f), vec4(0, 3, 0, 0));
    for (int side = 0; side < 2; ++side) {
        float s = side ? 1.0f : -1.0f, a = s * 0.225f, b = s * 0.335f;
        float part = side ? 2.0f : 1.0f;
        geo::addBox(m, vec3(std::fmin(a, b), 1.12f, -0.06f), vec3(std::fmax(a, b), 1.44f, 0.06f), vec4(part, 1, 0, 0));
        geo::addBox(m, vec3(std::fmin(a, b), 0.86f, -0.05f), vec3(std::fmax(a, b), 1.12f, 0.05f), vec4(part, 0, 0, 0));
    }
    vertexCount_ = (int)m.size() / geo::kFloatsPerVertex;

    std::mt19937 rng(7);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    struct Person {
        float order;
        glm::vec4 a, b;
    };
    std::vector<Person> people;
    const float cell = 0.55f;
    for (float z = 3.9f; z < 180.0f && (int)people.size() < kMax * 2; z += cell) {
        float halfWidth = std::min(70.0f, fanHalfWidth(z) - 0.8f);
        for (float x = -halfWidth; x < halfWidth; x += cell) {
            if (U(rng) > 0.85f) continue;
            float px = x + (U(rng) - 0.5f) * cell * 0.7f, pz = z + (U(rng) - 0.5f) * cell * 0.7f;
            glm::vec2 f = glm::normalize(glm::vec2(-px * 0.8f, -3.0f - pz));
            float turn = (U(rng) - 0.5f) * 0.6f;
            f = glm::vec2(f.x * std::cos(turn) - f.y * std::sin(turn), f.x * std::sin(turn) + f.y * std::cos(turn));
            float scale = 0.88f + 0.22f * U(rng);
            float dist = std::sqrt(px * px * 0.3f + pz * pz) + 3.0f * U(rng);
            people.push_back({dist, glm::vec4(px, pz, f.x, f.y), glm::vec4(scale, U(rng), U(rng), U(rng))});
        }
    }
    std::sort(people.begin(), people.end(), [](const Person& p, const Person& q) { return p.order < q.order; });
    if ((int)people.size() > kMax) people.resize(kMax);
    inst_.clear();
    for (size_t i = 0; i < people.size(); ++i) {
        inst_.push_back(people[i].a);
        inst_.push_back(people[i].b);
        if (i < (size_t)kDefault && U(rng) < 0.09f)
            phones_.emplace_back(people[i].a.x, 1.75f * people[i].b.x + 0.45f, people[i].a.y);
    }

    glGenVertexArrays(1, &vao_);
    glGenVertexArrays(1, &vaoNaive_);
    glGenBuffers(1, &vboMesh_);
    glGenBuffers(1, &vboInst_);
    glBindBuffer(GL_ARRAY_BUFFER, vboMesh_);
    glBufferData(GL_ARRAY_BUFFER, m.size() * sizeof(float), m.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
    glBufferData(GL_ARRAY_BUFFER, inst_.size() * sizeof(vec4), inst_.data(), GL_STATIC_DRAW);
    for (GLuint vao : {vao_, vaoNaive_}) {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vboMesh_);
        geo::setMeshAttribs();
    }
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst_);
    for (int a = 0; a < 2; ++a) {
        glEnableVertexAttribArray(3 + a);
        glVertexAttribPointer(3 + a, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(vec4), (void*)(a * sizeof(vec4)));
        glVertexAttribDivisor(3 + a, 1);
    }
    glBindVertexArray(0);
    std::cout << "[crowd] up to " << maxCount() << " people, " << vertexCount_ << " vertices each\n";
}

int Crowd::draw(Shader& s, int count, bool instanced, float jump, float arms, float sway) const {
    count = std::min(count, maxCount());
    s.set("uJump", jump);
    s.set("uArms", arms);
    s.set("uSway", sway);
    if (instanced) {
        glBindVertexArray(vao_);
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount_, count);
        return 1;
    }
    glBindVertexArray(vaoNaive_);
    for (int i = 0; i < count; ++i) {
        glVertexAttrib4fv(3, &inst_[2 * i].x);
        glVertexAttrib4fv(4, &inst_[2 * i + 1].x);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
    }
    return count;
}

void Crowd::release() {
    glDeleteBuffers(1, &vboMesh_);
    glDeleteBuffers(1, &vboInst_);
    glDeleteVertexArrays(1, &vao_);
    glDeleteVertexArrays(1, &vaoNaive_);
}
