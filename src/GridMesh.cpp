#include "GridMesh.h"
#include "Shader.h"

#include <cmath>
#include <cstdint>
#include <vector>

void GridMesh::init(int cells, float size) {
    cells_ = cells;
    size_ = size;
    std::vector<float> verts;
    std::vector<uint16_t> idx;
    for (int z = 0; z <= cells; ++z)
        for (int x = 0; x <= cells; ++x) {
            verts.push_back((float)x / cells - 0.5f);
            verts.push_back((float)z / cells - 0.5f);
        }
    for (int z = 0; z < cells; ++z)
        for (int x = 0; x < cells; ++x) {
            uint16_t a = (uint16_t)(z * (cells + 1) + x), b = a + 1, c = a + cells + 1, d = c + 1;
            idx.insert(idx.end(), {a, c, b, b, c, d});
        }
    indexCount_ = (int)idx.size();
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(uint16_t), idx.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void GridMesh::draw(Shader& s, const glm::vec3& center) const {
    const float cell = size_ / cells_;
    s.set("uGridCenter", glm::vec2(std::round(center.x / cell) * cell, std::round(center.z / cell) * cell));
    s.set("uGridSize", size_);
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_SHORT, nullptr);
}

void GridMesh::release() {
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteVertexArrays(1, &vao_);
}
