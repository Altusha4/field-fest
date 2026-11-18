#pragma once
#include "GridMesh.h"

class Terrain {
public:
    static float height(float x, float z);

    bool init();
    void draw(Shader& s, const glm::vec3& center) const;
    void release();

private:
    GridMesh grid_;
    GLuint tex_ = 0;
};
