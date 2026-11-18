#include "Performers.h"
#include "Geometry.h"
#include "MathUtil.h"
#include "Shader.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
using glm::vec3;
using mathx::kPi;
constexpr float kStageTop = 1.4f;

const vec3 kPivot[] = {
    {0.0f, 0.0f, 0.0f},
    {0.0f, 1.52f, 0.0f},
    {-0.31f, 1.44f, 0.0f},
    {-0.31f, 1.14f, 0.0f},
    {0.31f, 1.44f, 0.0f},
    {0.31f, 1.14f, 0.0f},
    {-0.11f, 0.88f, 0.0f},
    {0.11f, 0.88f, 0.0f},
};

glm::mat4 jointMatrix(const vec3& pivot, float rx, float rz) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), pivot);
    m = glm::rotate(m, rz, vec3(0, 0, 1));
    m = glm::rotate(m, rx, vec3(1, 0, 0));
    return glm::translate(m, -pivot);
}
}

void Performers::init() {
    auto body = [](Character& c, vec3 skin, vec3 shirt, vec3 jacket, vec3 sleeve, float armW, vec3 pants, vec3 shoes) {
        c.push_back({{-0.19f, 0.0f, -0.09f}, {-0.03f, 0.9f, 0.09f}, pants, LLeg});
        c.push_back({{0.03f, 0.0f, -0.09f}, {0.19f, 0.9f, 0.09f}, pants, RLeg});
        c.push_back({{-0.2f, 0.0f, -0.11f}, {-0.02f, 0.1f, 0.15f}, shoes, LLeg});
        c.push_back({{0.02f, 0.0f, -0.11f}, {0.2f, 0.1f, 0.15f}, shoes, RLeg});
        c.push_back({{-0.24f, 0.86f, -0.13f}, {0.24f, 1.48f, 0.13f}, shirt, Body});
        c.push_back({{-0.3f, 0.84f, -0.17f}, {-0.1f, 1.5f, 0.17f}, jacket, Body});
        c.push_back({{0.1f, 0.84f, -0.17f}, {0.3f, 1.5f, 0.17f}, jacket, Body});
        c.push_back({{-0.1f, 0.84f, -0.17f}, {0.1f, 1.5f, 0.0f}, jacket, Body});
        c.push_back({{-0.06f, 1.47f, -0.06f}, {0.06f, 1.57f, 0.06f}, skin, Head});
        c.push_back({{-0.12f, 1.55f, -0.12f}, {0.12f, 1.83f, 0.12f}, skin, Head});
        for (int s = -1; s <= 1; s += 2) {
            float x = 0.31f * s;
            Joint up = s < 0 ? LUpper : RUpper, fo = s < 0 ? LFore : RFore;
            c.push_back({{x - armW, 1.14f, -armW}, {x + armW, 1.48f, armW}, sleeve, up});
            c.push_back({{x - armW * 0.9f, 0.88f, -armW * 0.9f}, {x + armW * 0.9f, 1.15f, armW * 0.9f}, sleeve, fo});
            c.push_back({{x - 0.05f, 0.77f, -0.05f}, {x + 0.05f, 0.88f, 0.05f}, skin, fo});
        }
    };
    auto microphone = [](Character& c) {
        c.push_back({{0.29f, 0.63f, -0.022f}, {0.33f, 0.79f, 0.022f}, {0.06f, 0.06f, 0.07f}, RFore});
        c.push_back({{0.265f, 0.55f, -0.045f}, {0.355f, 0.64f, 0.045f}, {0.72f, 0.72f, 0.75f}, RFore});
    };

    Character a;
    const vec3 skinA(0.93f, 0.78f, 0.68f), bomber(0.24f, 0.21f, 0.16f);
    body(a, skinA, {0.9f, 0.9f, 0.87f}, bomber, bomber, 0.1f, {0.05f, 0.05f, 0.06f}, {0.07f, 0.07f, 0.07f});
    const vec3 blond(0.93f, 0.85f, 0.64f), beanie(0.05f, 0.05f, 0.06f), goggle(0.12f, 0.12f, 0.13f);
    a.push_back({{-0.13f, 1.6f, -0.13f}, {0.13f, 1.78f, 0.06f}, blond, Head});
    a.push_back({{-0.125f, 1.69f, 0.06f}, {0.125f, 1.76f, 0.13f}, blond, Head});
    a.push_back({{-0.135f, 1.74f, -0.135f}, {0.135f, 1.93f, 0.135f}, beanie, Head});
    a.push_back({{-0.115f, 1.8f, 0.12f}, {-0.01f, 1.89f, 0.18f}, goggle, Head});
    a.push_back({{0.01f, 1.8f, 0.12f}, {0.115f, 1.89f, 0.18f}, goggle, Head});
    a.push_back({{-0.14f, 1.83f, -0.14f}, {0.14f, 1.86f, 0.14f}, goggle, Head});
    a.push_back({{-0.335f, 0.66f, -0.015f}, {-0.315f, 0.78f, 0.015f}, skinA, LFore});
    a.push_back({{-0.305f, 0.66f, -0.015f}, {-0.285f, 0.78f, 0.015f}, skinA, LFore});
    microphone(a);

    Character b;
    const vec3 skinB(0.9f, 0.74f, 0.62f), leather(0.07f, 0.07f, 0.08f), cream(0.88f, 0.82f, 0.66f);
    body(b, skinB, {0.05f, 0.05f, 0.05f}, leather, leather, 0.075f, {0.12f, 0.13f, 0.19f}, {0.06f, 0.06f, 0.06f});
    const vec3 brown(0.34f, 0.22f, 0.13f);
    b.push_back({{-0.13f, 1.76f, -0.13f}, {0.13f, 1.87f, 0.11f}, brown, Head});
    b.push_back({{-0.135f, 1.5f, -0.16f}, {0.135f, 1.84f, -0.08f}, brown, Head});
    b.push_back({{-0.135f, 1.62f, -0.1f}, {0.135f, 1.82f, 0.04f}, brown, Head});
    b.push_back({{-0.13f, 1.67f, 0.12f}, {0.13f, 1.73f, 0.155f}, {0.02f, 0.02f, 0.02f}, Head});
    for (int s = -1; s <= 1; s += 2) {
        float x = 0.31f * s;
        Joint up = s < 0 ? LUpper : RUpper, fo = s < 0 ? LFore : RFore;
        b.push_back({{x - 0.03f, 1.14f, 0.075f}, {x + 0.03f, 1.48f, 0.085f}, cream, up});
        b.push_back({{x - 0.03f, 0.88f, 0.068f}, {x + 0.03f, 1.15f, 0.078f}, cream, fo});
        b.push_back({{s < 0 ? -0.3f : 0.1f, 1.5f, -0.03f}, {s < 0 ? -0.1f : 0.3f, 1.52f, 0.03f}, cream, Body});
    }
    microphone(b);

    Character dj;
    const vec3 hoodie(0.33f, 0.1f, 0.52f);
    body(dj, {0.62f, 0.44f, 0.32f}, hoodie, hoodie, hoodie, 0.08f, {0.1f, 0.1f, 0.12f}, {0.85f, 0.85f, 0.85f});
    dj.push_back({{-0.13f, 1.76f, -0.13f}, {0.13f, 1.86f, 0.12f}, {0.04f, 0.03f, 0.03f}, Head});
    dj.push_back({{-0.15f, 1.62f, -0.05f}, {-0.12f, 1.74f, 0.05f}, {0.08f, 0.08f, 0.09f}, Head});
    dj.push_back({{0.12f, 1.62f, -0.05f}, {0.15f, 1.74f, 0.05f}, {0.08f, 0.08f, 0.09f}, Head});
    dj.push_back({{-0.15f, 1.86f, -0.02f}, {0.15f, 1.89f, 0.02f}, {0.08f, 0.08f, 0.09f}, Head});

    Character s;
    const vec3 skinS(0.86f, 0.68f, 0.54f), white(0.93f, 0.93f, 0.92f), navy(0.06f, 0.07f, 0.12f), black(0.03f, 0.03f, 0.03f);
    body(s, skinS, white, white, white, 0.075f, {0.1f, 0.12f, 0.2f}, {0.05f, 0.05f, 0.05f});
    s.push_back({{-0.13f, 1.74f, -0.13f}, {0.13f, 1.88f, 0.1f}, black, Head});
    s.push_back({{-0.13f, 1.6f, -0.14f}, {0.13f, 1.8f, -0.06f}, black, Head});
    s.push_back({{-0.135f, 1.66f, -0.06f}, {0.135f, 1.78f, 0.02f}, black, Head});
    s.push_back({{-0.125f, 1.67f, 0.12f}, {-0.01f, 1.73f, 0.15f}, {0.05f, 0.05f, 0.06f}, Head});
    s.push_back({{0.01f, 1.67f, 0.12f}, {0.125f, 1.73f, 0.15f}, {0.05f, 0.05f, 0.06f}, Head});
    s.push_back({{-0.13f, 1.725f, 0.125f}, {0.13f, 1.735f, 0.155f}, {0.6f, 0.6f, 0.62f}, Head});
    s.push_back({{-0.05f, 1.36f, 0.13f}, {0.05f, 1.38f, 0.145f}, {0.8f, 0.8f, 0.82f}, Body});
    s.push_back({{-0.245f, 0.84f, -0.14f}, {0.245f, 0.9f, 0.145f}, black, Body});
    s.push_back({{-0.04f, 0.845f, 0.145f}, {0.04f, 0.895f, 0.16f}, {0.75f, 0.75f, 0.78f}, Body});
    s.push_back({{-0.42f, 1.42f, -0.2f}, {0.42f, 1.53f, 0.17f}, navy, Body});
    s.push_back({{-0.42f, 0.55f, -0.22f}, {0.42f, 1.48f, -0.14f}, navy, Body});
    s.push_back({{-0.42f, 0.6f, -0.16f}, {-0.3f, 1.47f, 0.17f}, navy, Body});
    s.push_back({{0.3f, 0.6f, -0.16f}, {0.42f, 1.47f, 0.17f}, navy, Body});
    s.push_back({{-0.3f, 0.75f, 0.13f}, {-0.2f, 1.45f, 0.18f}, navy, Body});
    s.push_back({{0.2f, 0.75f, 0.13f}, {0.3f, 1.45f, 0.18f}, navy, Body});
    microphone(s);

    chars_ = {a, b, dj, s};
    size_t boxes = a.size() + b.size() + dj.size() + s.size();
    capacity_ = (int)boxes * 36;
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, capacity_ * geo::kFloatsPerVertex * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    geo::setMeshAttribs();
    glBindVertexArray(0);
}

void Performers::build(const Character& c, const Pose& p) {
    glm::mat4 M[JointCount];
    M[Body] = glm::translate(glm::mat4(1.0f), p.pos + vec3(0.0f, p.bounce, 0.0f));
    M[Body] = glm::rotate(M[Body], p.yaw, vec3(0, 1, 0));
    M[Head] = M[Body] * jointMatrix(kPivot[Head], p.nod, p.turn);
    M[LUpper] = M[Body] * jointMatrix(kPivot[LUpper], p.lUpper.x, p.lUpper.y);
    M[LFore] = M[LUpper] * jointMatrix(kPivot[LFore], p.lFore.x, p.lFore.y);
    M[RUpper] = M[Body] * jointMatrix(kPivot[RUpper], p.rUpper.x, p.rUpper.y);
    M[RFore] = M[RUpper] * jointMatrix(kPivot[RFore], p.rFore.x, p.rFore.y);
    M[LLeg] = M[Body] * jointMatrix(kPivot[LLeg], p.legSwing, 0.0f);
    M[RLeg] = M[Body] * jointMatrix(kPivot[RLeg], -p.legSwing, 0.0f);

    for (const Part& part : c) geo::addBox(verts_, part.mn, part.mx, glm::vec4(part.color, 0.0f), M[part.joint]);
}

void Performers::update(float t, float beats, float energy, Cast cast) {
    bool rappers = cast == Cast::Duo;
    verts_.clear();
    float e = 0.3f + 0.7f * energy;
    float beatPhase = beats - std::floor(beats);
    spotCount_ = 0;

    if (rappers) {
        for (int i = 0; i < 2; ++i) {
            Pose p;
            float side = i == 0 ? -1.0f : 1.0f, ph = i * 2.1f;
            float walk = std::sin(0.3f * t + ph);
            float speed = std::fabs(std::cos(0.3f * t + ph));
            p.pos = vec3(side * 2.8f + 1.8f * walk, kStageTop, -1.4f);
            p.yaw = 0.35f * std::cos(0.3f * t + ph) * side * -1.0f + 0.2f * std::sin(0.17f * t + ph);
            p.legSwing = 0.45f * std::sin(t * 5.0f + ph) * speed;
            p.bounce = 0.07f * e * std::fabs(std::sin(kPi * beats));
            p.nod = 0.13f * e * std::sin(2.0f * kPi * beats);
            p.turn = 0.25f * std::sin(0.4f * t + ph);
            p.rUpper = {-0.95f, -0.45f};
            p.rFore = {-1.55f, 0.0f};
            if (i == 0) {
                p.lUpper = {-0.2f, -2.25f - 0.18f * e * std::sin(2.0f * kPi * beats)};
                p.lFore = {0.0f, -0.4f};
            } else {
                p.lUpper = {-0.5f - 0.7f * e * (0.5f + 0.5f * std::sin(kPi * beats)), -0.25f};
                p.lFore = {-0.6f, 0.0f};
            }
            build(chars_[i], p);
            spot_[spotCount_++] = p.pos + vec3(0.0f, 2.6f, 2.0f);
        }
    }
    if (cast == Cast::Singer) {
        Pose p;
        float walk = std::sin(0.22f * t), speed = std::fabs(std::cos(0.22f * t));
        p.pos = vec3(3.2f * walk, kStageTop, -1.2f);
        p.yaw = -0.3f * std::cos(0.22f * t) * walk + 0.15f * std::sin(0.13f * t);
        p.legSwing = 0.35f * std::sin(t * 4.0f) * speed;
        p.bounce = 0.04f * e * std::fabs(std::sin(kPi * beats));
        p.nod = 0.06f * e * std::sin(2.0f * kPi * beats);
        p.turn = 0.3f * std::sin(0.35f * t);
        p.rUpper = {-0.95f, -0.45f};
        p.rFore = {-1.55f, 0.0f};
        float open = 0.5f + 0.5f * std::sin(kPi * beats * 0.5f);
        p.lUpper = {-0.6f - 0.6f * open * e, -0.5f - 0.6f * open};
        p.lFore = {-0.3f, 0.0f};
        build(chars_[3], p);
        spot_[spotCount_++] = p.pos + vec3(0.0f, 2.6f, 2.0f);
    }
    Pose d;
    d.pos = vec3(0.0f, kStageTop, -4.75f);
    d.bounce = 0.04f * e * std::fabs(std::sin(kPi * beats));
    d.nod = 0.2f * e * std::sin(2.0f * kPi * beats);
    d.lUpper = {-0.75f, 0.15f};
    d.lFore = {-0.45f, 0.0f};
    bool handUp = energy > 0.8f && std::fmod(beats, 8.0f) >= 4.0f;
    d.rUpper = handUp ? glm::vec2(-0.2f, -2.5f + 0.15f * std::sin(2.0f * kPi * beatPhase)) : glm::vec2(-0.75f, -0.15f);
    d.rFore = handUp ? glm::vec2(0.0f, -0.2f) : glm::vec2(-0.45f, 0.0f);
    build(chars_[2], d);
    if (spotCount_ < 2) spot_[spotCount_++] = d.pos + vec3(0.0f, 2.2f, 1.6f);

    count_ = std::min((int)verts_.size() / geo::kFloatsPerVertex, capacity_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count_ * geo::kFloatsPerVertex * sizeof(float), verts_.data());
}

void Performers::draw(Shader&) const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, count_);
}

void Performers::release() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}
