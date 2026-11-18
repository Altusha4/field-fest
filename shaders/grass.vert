#version 330 core

layout(location = 0) in vec2 aBlade;
layout(location = 1) in vec4 aInst0;
layout(location = 2) in vec4 aInst1;

#include "common.glsl"

uniform mat4  uViewProj;
uniform vec2  uCenter;
uniform float uFieldSize;
uniform float uWind;
uniform vec3  uGrassDark, uGrassLight, uGrassDry;
uniform float uGrassHeight, uGrassCoverage;

out float vT;
out vec3  vColor;
out vec3  vWorld;
out vec3  vNormal;

void main() {
    vec2 local = aInst0.xy;
    vec2 base  = local + uFieldSize * floor((uCenter - local) / uFieldSize + 0.5);

    vec2  q     = abs(base - uCenter);
    float edge  = 1.0 - smoothstep(uFieldSize * 0.5 - 10.0, uFieldSize * 0.5, max(q.x, q.y));
    float h     = aInst0.z * mix(1.0, 0.22, crowdMask(base)) * edge * uGrassHeight;
    h *= step(aInst1.y, uGrassCoverage);
    float ground = terrainHeight(base);
    if (uLocation == 1) h *= smoothstep(uWaterLevel + 0.5, uWaterLevel + 1.6, ground);
    float angle = aInst0.w;
    float width = aInst1.x;
    float rnd   = aInst1.y;
    vec2 facing = vec2(cos(angle), sin(angle));
    float flowerHash = fract(rnd * 37.71);
    bool flower = uLocation == 0 && flowerHash < 0.04;
    vec2 side   = vec2(-facing.y, facing.x);
    float t     = aBlade.y;

    vec2  windDir = normalize(vec2(1.0, 0.35));
    float gust    = 0.5 + 0.5 * sin(dot(base, windDir) * 0.32 - uTime * 2.0);
    gust *= gust;
    float flutter = 0.12 * sin(uTime * 5.0 + rnd * 30.0 + base.x);
    vec2  offset  = windDir * (0.1 + (0.9 * gust + flutter) * uWind) + facing * 0.18;

    vec2  fromStage = base - vec2(0.0, -2.0);
    float r     = length(fromStage);
    float front = uKickAge * 38.0;
    float x     = (r - front) / 3.0;
    float ring  = exp(-x * x) * exp(-uKickAge * 1.5);
    offset += fromStage / max(r, 1e-3) * ring * uKickPower * 1.3;
    offset *= min(1.0, 1.25 / max(length(offset), 1e-3));
    float bend = length(offset);

    vec3 p;
    float taper = flower ? (t > 0.8 ? 2.6 : 1.0 - 0.85 * t) : 1.0 - 0.85 * t;
    p.xz = base + side * aBlade.x * width * taper + offset * h * t * t * 0.85;
    p.y  = ground + h * t * (1.0 - 0.32 * bend * bend * t);

    vT     = t;
    vWorld = p;
    vNormal = normalize(vec3(facing.x, 0.0, facing.y) * 0.5 + vec3(0.0, 0.5 + 0.5 * t, 0.0)
                        - vec3(offset.x, 0.0, offset.y) * 0.2 * t);

    float patchN = 0.5 + 0.5 * sin(base.x * 0.21) * sin(base.y * 0.17 + 1.3);
    vColor = mix(mix(uGrassDark, uGrassLight, t), uGrassDry * mix(0.5, 1.0, t), patchN * aInst1.z * 0.9);
    if (flower && t > 0.79) {
        vec3 petals[4] = vec3[4](vec3(1.0, 0.85, 0.15), vec3(0.95, 0.95, 0.95), vec3(0.65, 0.35, 0.95), vec3(0.95, 0.25, 0.25));
        vColor = petals[int(fract(rnd * 91.3) * 4.0)] * 1.4;
    }

    gl_Position = uViewProj * vec4(p, 1.0);
}
