#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aPart;
layout(location = 3) in vec4 aInst0;
layout(location = 4) in vec4 aInst1;

#include "common.glsl"

uniform mat4  uViewProj;
uniform float uJump;
uniform float uArms;
uniform float uSway;

out vec3 vWorld;
out vec3 vNormal;
out vec3 vColor;

vec3 rotZ(vec3 v, float a) {
    float c = cos(a), s = sin(a);
    return vec3(c * v.x - s * v.y, s * v.x + c * v.y, v.z);
}

vec3 hsv2rgb(vec3 c) {
    vec3 p = abs(fract(c.xxx + vec3(0.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
    return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

void main() {
    vec3 p = aPos, n = aNormal;
    float phase = aInst1.z;

    if (aPart.x > 0.5) {
        float side  = aPart.x < 1.5 ? -1.0 : 1.0;
        float raise = clamp(uArms * 1.5 - aInst1.w, 0.0, 1.0);
        float pump  = 0.3 * sin(6.2831 * (uBeats + phase));
        float a     = side * raise * (2.45 + pump);
        vec3 pivot  = vec3(0.28 * side, 1.42, 0.0);
        p = pivot + rotZ(p - pivot, a);
        n = rotZ(n, a);
    }
    float lean = uSway * 0.08 * sin(6.2831 * (uBeats * 0.5 + phase));
    p = rotZ(p, lean) * aInst1.x;
    n = rotZ(n, lean);

    float f = fract(uBeats + phase * 0.12);
    float jump = uJump * 4.0 * f * (1.0 - f) * (0.7 + 0.6 * aInst1.w);

    vec2 F = aInst0.zw, R = vec2(F.y, -F.x);
    vec3 world = vec3(aInst0.x + R.x * p.x + F.x * p.z,
                      terrainHeight(aInst0.xy) + p.y + jump,
                      aInst0.y + R.y * p.x + F.y * p.z);
    vNormal = vec3(R.x * n.x + F.x * n.z, n.y, R.y * n.x + F.y * n.z);
    vWorld = world;

    float kind = aPart.y;
    vColor = kind < 0.5 ? mix(vec3(0.92, 0.72, 0.58), vec3(0.38, 0.25, 0.17), fract(phase * 7.3))
           : kind < 1.5 ? hsv2rgb(vec3(aInst1.y, 0.6, 0.8))
           : kind < 2.5 ? mix(vec3(0.06, 0.08, 0.16), vec3(0.24, 0.21, 0.18), fract(phase * 3.1))
           :              mix(vec3(0.04, 0.03, 0.02), vec3(0.5, 0.33, 0.12), fract(phase * 5.7));
    gl_Position = uViewProj * vec4(world, 1.0);
}
