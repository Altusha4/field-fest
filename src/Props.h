#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class Props {
public:
    void init();
    void draw(Shader& s, int location) const;
    void release();

private:
    struct Kind {
        GLuint vao = 0, vboMesh = 0, vboInst = 0;
        int vertexCount = 0, count = 0;
    };
    void upload(Kind& k, const std::vector<float>& mesh, const std::vector<glm::vec4>& inst);
    Kind kinds_[3];
};
