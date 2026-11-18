#version 330 core
layout(location = 0) in vec2 aGrid;

#include "common.glsl"

uniform mat4  uViewProj;
uniform vec2  uGridCenter;
uniform float uGridSize;

out vec3 vWorld;
out vec3 vNormal;

const vec2  kDir[3]   = vec2[3](vec2(0.0, -1.0), vec2(0.6, -0.8), vec2(-0.7, -0.7));
const float kAmp[3]   = float[3](0.14, 0.08, 0.04);
const float kFreq[3]  = float[3](0.35, 0.62, 1.3);
const float kSpeed[3] = float[3](1.1, 1.6, 2.3);

void main() {
    vec2 xz = uGridCenter + aGrid * uGridSize;
    float h = 0.0;
    vec2 grad = vec2(0.0);
    for (int i = 0; i < 3; ++i) {
        float ph = dot(xz, kDir[i]) * kFreq[i] - uTime * kSpeed[i];
        h += kAmp[i] * sin(ph);
        grad += kAmp[i] * kFreq[i] * cos(ph) * kDir[i];
    }
    vWorld = vec3(xz.x, uWaterLevel + h, xz.y);
    vNormal = normalize(vec3(-grad.x, 1.0, -grad.y));
    gl_Position = uViewProj * vec4(vWorld, 1.0);
}
