#pragma once
#include <glm/glm.hpp>

class Shader;

namespace location {

enum Id { Meadow, Beach, Winter, Count };

struct Location {
    const char* name;
    const char* description;
    glm::vec3 groundColor;
    float groundRecolor;
    glm::vec3 grassDark, grassLight, grassDry;
    float grassHeight;
    float grassCoverage;
    glm::vec3 horizonTint, zenithTint, sunTint, ambientTint;
    bool water, snow;
    float aurora;
};

void set(int id);
int current();
const Location& get(int id);
inline const Location& cur() { return get(current()); }

constexpr float kWaterLevel = -0.35f;

void apply(Shader& s);

}
