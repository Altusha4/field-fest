#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

#ifndef SHADER_DIR
#define SHADER_DIR "shaders"
#endif

namespace {

bool fileExists(const std::string& p) {
    std::ifstream f(p);
    return f.good();
}

bool readFile(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

bool expandIncludes(const std::string& src, const std::string& fromFile, std::string& out) {
    std::istringstream in(src);
    std::ostringstream res;
    std::string line;
    while (std::getline(in, line)) {
        auto pos = line.find("#include");
        if (pos != std::string::npos && line.find_first_not_of(" \t") == pos) {
            auto a = line.find('"'), b = line.rfind('"');
            if (a == std::string::npos || b <= a) {
                std::cerr << "[shader] bad #include in " << fromFile << ": " << line << "\n";
                return false;
            }
            std::string inc = line.substr(a + 1, b - a - 1);
            std::string incSrc;
            if (!readFile(resolveShaderPath(inc), incSrc)) {
                std::cerr << "[shader] cannot open included file '" << inc << "' (from " << fromFile << ")\n";
                return false;
            }
            res << "// ---- begin " << inc << "\n" << incSrc << "\n// ---- end " << inc << "\n";
        } else {
            res << line << "\n";
        }
    }
    out = res.str();
    return true;
}

GLuint compile(GLenum type, const std::string& file) {
    std::string raw, src;
    std::string path = resolveShaderPath(file);
    if (!readFile(path, raw)) {
        std::cerr << "[shader] cannot open '" << file << "' (looked in " << SHADER_DIR << " and ./shaders)\n";
        return 0;
    }
    if (!expandIncludes(raw, file, src)) return 0;
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetShaderInfoLog(s, (GLsizei)log.size(), nullptr, log.data());
        std::cerr << "[shader] COMPILE ERROR in " << path << ":\n" << log.data() << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

}

std::string resolveShaderPath(const std::string& file) {
    std::string src = std::string(SHADER_DIR) + "/" + file;
    if (fileExists(src)) return src;
    return "shaders/" + file;
}

Shader::~Shader() {
    if (program_) glDeleteProgram(program_);
}

bool Shader::load(const std::string& vertFile, const std::string& fragFile) {
    vertFile_ = vertFile;
    fragFile_ = fragFile;
    return reload();
}

bool Shader::reload() {
    GLuint vs = compile(GL_VERTEX_SHADER, vertFile_);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fragFile_);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        std::cerr << "[shader] keeping previous program for " << vertFile_ << " / " << fragFile_ << "\n";
        return false;
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::cerr << "[shader] LINK ERROR " << vertFile_ << " + " << fragFile_ << ":\n" << log << "\n";
        glDeleteProgram(p);
        return false;
    }
    if (program_) glDeleteProgram(program_);
    program_ = p;
    cache_.clear();
    std::cout << "[shader] loaded " << vertFile_ << " + " << fragFile_ << "\n";
    return true;
}

GLint Shader::loc(const char* name) {
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;
    GLint l = glGetUniformLocation(program_, name);
    cache_[name] = l;
    return l;
}

void Shader::set(const char* n, int v) { glUniform1i(loc(n), v); }
void Shader::set(const char* n, float v) { glUniform1f(loc(n), v); }
void Shader::set(const char* n, const glm::vec2& v) { glUniform2fv(loc(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::vec3& v) { glUniform3fv(loc(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::vec4& v) { glUniform4fv(loc(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::mat4& m) { glUniformMatrix4fv(loc(n), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::setVec3Array(const char* n, const glm::vec3* v, int count) {
    glUniform3fv(loc(n), count, glm::value_ptr(v[0]));
}
