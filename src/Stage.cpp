#include "Stage.h"
#include "Geometry.h"
#include "Shader.h"

#include <algorithm>
#include <vector>

void Stage::init() {
    using glm::vec3;
    using glm::vec4;
    std::vector<float> v;
    const vec4 black(0.05f, 0.05f, 0.06f, 0.0f), metal(0.2f, 0.2f, 0.22f, 0.0f), grey(0.35f, 0.35f, 0.36f, 0.0f);
    const vec4 screen(0.0f, 0.0f, 0.0f, 1.0f), strip(0.0f, 0.0f, 0.0f, 2.0f);

    geo::addBox(v, vec3(-10, -0.6f, -9), vec3(10, 1.4f, 0), black);
    geo::addBox(v, vec3(-10, 1.22f, 0), vec3(10, 1.4f, 0.06f), strip);
    geo::addBox(v, vec3(-8.6f, 1.4f, -8.9f), vec3(8.6f, 8.4f, -8.5f), black);
    geo::addBox(v, vec3(kScreen[0], kScreen[1], -8.5f), vec3(kScreen[2], kScreen[3], -8.42f), screen);
    geo::addBox(v, vec3(-2.2f, 1.4f, -4.2f), vec3(2.2f, 2.5f, -3.2f), black);
    geo::addBox(v, vec3(-2.2f, 1.6f, -3.2f), vec3(2.2f, 2.3f, -3.15f), strip);

    for (float x : {-10.2f, 9.6f})
        for (float z : {-1.4f, -8.6f}) geo::addBox(v, vec3(x, -0.6f, z), vec3(x + 0.6f, 10.4f, z + 0.6f), metal);
    geo::addBox(v, vec3(-10.2f, 9.8f, -1.4f), vec3(10.2f, 10.4f, -0.8f), metal);
    geo::addBox(v, vec3(-10.2f, 9.8f, -8.6f), vec3(10.2f, 10.4f, -8.0f), metal);
    geo::addBox(v, vec3(-10.2f, 9.8f, -8.0f), vec3(-9.6f, 10.4f, -1.4f), metal);
    geo::addBox(v, vec3(9.6f, 9.8f, -8.0f), vec3(10.2f, 10.4f, -1.4f), metal);
    geo::addBox(v, vec3(-10.6f, 10.4f, -9.2f), vec3(10.6f, 10.7f, -0.4f), black);

    for (int i = 0; i < kSpotFixtures; ++i) {
        float x = spotX(i);
        geo::addBox(v, vec3(x - 0.25f, 9.3f, -1.35f), vec3(x + 0.25f, 9.8f, -0.85f), black);
    }
    for (float s : {-1.0f, 1.0f}) {
        float a = s * 10.6f, b = s * 13.6f;
        geo::addBox(v, vec3(std::min(a, b), -0.6f, -2.4f), vec3(std::max(a, b), 4.4f, 0.2f), black);
        float c = s * 11.2f, d = s * 12.6f;
        geo::addBox(v, vec3(std::min(c, d), 6.0f, -1.6f), vec3(std::max(c, d), 9.5f, -0.6f), black);
    }
    geo::addBox(v, vec3(-12, -0.3f, 2.6f), vec3(12, 1.1f, 2.75f), grey);

    count_ = (int)v.size() / geo::kFloatsPerVertex;
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    geo::setMeshAttribs();
    glBindVertexArray(0);
}

void Stage::draw(Shader& s, const glm::vec3& accent, float showOn) const {
    s.set("uScreenRect", glm::vec4(kScreen[0], kScreen[1], kScreen[2], kScreen[3]));
    s.set("uAccent", accent);
    s.set("uShowOn", showOn);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, count_);
}

void Stage::release() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}
