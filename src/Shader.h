#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader() = default;
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    ~Shader();

    bool load(const std::string& vertFile, const std::string& fragFile);
    bool reload();
    void use() const { glUseProgram(program_); }
    bool valid() const { return program_ != 0; }

    void set(const char* name, int v);
    void set(const char* name, float v);
    void set(const char* name, const glm::vec2& v);
    void set(const char* name, const glm::vec3& v);
    void set(const char* name, const glm::vec4& v);
    void set(const char* name, const glm::mat4& m);
    void setVec3Array(const char* name, const glm::vec3* v, int count);

private:
    GLint loc(const char* name);
    std::string vertFile_, fragFile_;
    GLuint program_ = 0;
    std::unordered_map<std::string, GLint> cache_;
};

std::string resolveShaderPath(const std::string& file);
