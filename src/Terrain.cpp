#include "Terrain.h"
#include "Location.h"
#include "MathUtil.h"
#include "Shader.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

using mathx::smoothstep;

float Terrain::height(float x, float z) {
    float hills = 1.4f * std::sin(x * 0.07f) * std::cos(z * 0.06f) + 0.6f * std::sin(x * 0.19f + z * 0.13f) +
                  0.25f * std::cos(z * 0.31f - x * 0.11f);
    float d = std::sqrt(x * x + (z - 6.0f) * (z - 6.0f));
    float far = smoothstep(22.0f, 70.0f, d), bowl = smoothstep(8.0f, 45.0f, d);
    switch (location::current()) {
        case location::Beach: {
            float h = 0.45f * hills * far + 1.2f * bowl;
            float shore = std::fmax(1.0f - smoothstep(-34.0f, -13.0f, z), 1.0f - smoothstep(-75.0f, -42.0f, x));
            return h + (-3.2f - h) * shore;
        }
        case location::Winter: return 1.9f * hills * far + 2.2f * bowl;
        default: return hills * far + 2.2f * bowl;
    }
}

namespace {
float hash2(int x, int y) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFFFF) / 16777215.0f;
}
float vnoise(float x, float y, int period) {
    int xi = (int)std::floor(x), yi = (int)std::floor(y);
    float fx = x - xi, fy = y - yi;
    fx = fx * fx * (3 - 2 * fx);
    fy = fy * fy * (3 - 2 * fy);
    auto w = [period](int v) { return mathx::wrap(v, period); };
    float a = hash2(w(xi), w(yi)), b = hash2(w(xi + 1), w(yi));
    float c = hash2(w(xi), w(yi + 1)), d = hash2(w(xi + 1), w(yi + 1));
    return (a + (b - a) * fx) * (1 - fy) + (c + (d - c) * fx) * fy;
}
float mixf(float a, float b, float t) { return a + (b - a) * t; }
}

bool Terrain::init() {
    grid_.init(200, 220.0f);

    const int N = 512;
    std::vector<uint8_t> px(N * N * 3);
    for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x) {
            float u = (float)x / N, v = (float)y / N;
            float n = 0.5f * vnoise(u * 8, v * 8, 8) + 0.25f * vnoise(u * 16, v * 16, 16) +
                      0.15f * vnoise(u * 32, v * 32, 32) + 0.10f * vnoise(u * 64, v * 64, 64);
            float speck = vnoise(u * 128, v * 128, 128);
            float dry = std::pow(vnoise(u * 4 + 3, v * 4 + 7, 4), 2.0f);
            float r = mixf(0.055f, 0.075f, n), g = mixf(0.075f, 0.13f, n), b = mixf(0.03f, 0.04f, n);
            r = mixf(r, 0.13f, dry * 0.6f);
            g = mixf(g, 0.11f, dry * 0.6f);
            b = mixf(b, 0.055f, dry * 0.6f);
            float s = 0.75f + 0.5f * speck;
            uint8_t* p = &px[(y * N + x) * 3];
            p[0] = (uint8_t)std::fmin(255.0f, r * s * 255.0f);
            p[1] = (uint8_t)std::fmin(255.0f, g * s * 255.0f);
            p[2] = (uint8_t)std::fmin(255.0f, b * s * 255.0f);
        }
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, N, N, 0, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "[terrain] OpenGL error while creating ground texture\n";
        return false;
    }
    return true;
}

void Terrain::draw(Shader& s, const glm::vec3& center) const {
    s.set("uGroundTex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_);
    grid_.draw(s, center);
}

void Terrain::release() {
    glDeleteTextures(1, &tex_);
    grid_.release();
}
