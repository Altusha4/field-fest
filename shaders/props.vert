#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec4 aInst;

#include "common.glsl"

uniform mat4 uViewProj;

out vec3 vWorld;
out vec3 vNormal;
out vec3 vColor;

void main() {
    float c = cos(aInst.w), s = sin(aInst.w);
    vec3 p = aPos * aInst.z;
    vec3 n = aNormal;
    p.xz = vec2(c * p.x + s * p.z, -s * p.x + c * p.z);
    n.xz = vec2(c * n.x + s * n.z, -s * n.x + c * n.z);
    float sway = 0.02 * p.y * p.y * (0.6 + 0.4 * sin(uTime * 1.3 + aInst.x * 0.2));
    p.x += sway * 0.8;
    p.z += sway * 0.3;
    vWorld = vec3(aInst.x, terrainHeight(aInst.xy), aInst.y) + p;
    vNormal = n;
    vColor = aColor.rgb;
    gl_Position = uViewProj * vec4(vWorld, 1.0);
}
