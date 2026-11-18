#version 330 core
layout(location = 0) in vec2 aGrid;

#include "common.glsl"

uniform mat4  uViewProj;
uniform vec2  uGridCenter;
uniform float uGridSize;

out vec3 vWorld;
out vec3 vNormal;

void main() {
    vec2 xz = uGridCenter + aGrid * uGridSize;
    float e = 0.1;
    vNormal = normalize(vec3(terrainHeight(xz - vec2(e, 0.0)) - terrainHeight(xz + vec2(e, 0.0)),
                             2.0 * e,
                             terrainHeight(xz - vec2(0.0, e)) - terrainHeight(xz + vec2(0.0, e))));
    vWorld = vec3(xz.x, terrainHeight(xz), xz.y);
    gl_Position = uViewProj * vec4(vWorld, 1.0);
}
