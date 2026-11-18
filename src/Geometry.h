#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace geo {

constexpr int kFloatsPerVertex = 10;

inline void addBox(std::vector<float>& v, glm::vec3 mn, glm::vec3 mx, glm::vec4 extra) {
    glm::vec3 c[8];
    for (int i = 0; i < 8; ++i) c[i] = glm::vec3(i & 1 ? mx.x : mn.x, i & 2 ? mx.y : mn.y, i & 4 ? mx.z : mn.z);
    const int face[6][4] = {{1, 3, 7, 5}, {0, 4, 6, 2}, {2, 6, 7, 3}, {0, 1, 5, 4}, {4, 5, 7, 6}, {0, 2, 3, 1}};
    const glm::vec3 normal[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    for (int f = 0; f < 6; ++f) {
        const int tri[6] = {0, 1, 2, 0, 2, 3};
        for (int k : tri) {
            const glm::vec3& p = c[face[f][k]];
            v.insert(v.end(), {p.x, p.y, p.z, normal[f].x, normal[f].y, normal[f].z, extra.x, extra.y, extra.z, extra.w});
        }
    }
}

inline void addBox(std::vector<float>& v, glm::vec3 mn, glm::vec3 mx, glm::vec4 extra, const glm::mat4& m) {
    std::vector<float> box;
    addBox(box, mn, mx, extra);
    glm::mat3 n(m);
    for (size_t i = 0; i < box.size(); i += kFloatsPerVertex) {
        glm::vec3 p = glm::vec3(m * glm::vec4(box[i], box[i + 1], box[i + 2], 1.0f));
        glm::vec3 q = glm::normalize(n * glm::vec3(box[i + 3], box[i + 4], box[i + 5]));
        v.insert(v.end(), {p.x, p.y, p.z, q.x, q.y, q.z, extra.x, extra.y, extra.z, extra.w});
    }
}

inline void setMeshAttribs() {
    const GLsizei stride = kFloatsPerVertex * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
}

}
