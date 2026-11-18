#version 330 core
#include "common.glsl"

in vec3 vWorld;
in vec3 vNormal;
out vec4 FragColor;

uniform sampler2D uGroundTex;
uniform vec3  uGroundColor;
uniform float uGroundRecolor;

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec3 tex = texture(uGroundTex, vWorld.xz * 0.3).rgb * 1.1;
    float detail = dot(tex, vec3(0.3, 0.59, 0.11)) / 0.1;
    vec3 albedo = mix(tex, uGroundColor * mix(0.75, 1.15, clamp(detail - 0.5, 0.0, 1.0)), uGroundRecolor);
    if (uLocation == 1)
        albedo *= mix(0.55, 1.0, smoothstep(uWaterLevel, uWaterLevel + 0.8, vWorld.y));
    vec3 c = shade(albedo, normalize(vNormal), vWorld, 0.0);
    if (uLocation == 2) {
        vec2 cell = floor(vWorld.xz * 25.0);
        float h = hash12(cell);
        float glint = step(0.993, h) * pow(0.5 + 0.5 * sin(dot(normalize(vWorld - uCamPos), vec3(40.0)) + h * 50.0), 8.0);
        c += vec3(0.9, 0.95, 1.0) * glint * 1.5 * (1.0 - fogFactor(vWorld));
    }
    FragColor = vec4(c, 1.0);
}
