#pragma once
#include "GridMesh.h"

class Water {
public:
    void init() { grid_.init(160, 360.0f); }
    void draw(Shader& s, const glm::vec3& center) const { grid_.draw(s, center); }
    void release() { grid_.release(); }

private:
    GridMesh grid_;
};
