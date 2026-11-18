#version 330 core
#include "common.glsl"

in float vT;
in vec3  vColor;
in vec3  vWorld;
in vec3  vNormal;
out vec4 FragColor;

uniform float uBeamGain;

void main() {
    vec3 V = normalize(uCamPos - vWorld);
    float facing = abs(dot(normalize(vNormal), V));
    float haze = 0.7 + 0.3 * sin(vWorld.x * 0.35 + uTime * 0.7) * sin(vWorld.z * 0.3 - uTime * 0.5 + vWorld.y * 0.4);
    float a = facing * facing * pow(1.0 - vT, 1.6) * haze * (1.0 - fogFactor(vWorld));
    a *= smoothstep(1.5, 8.0, distance(vWorld, uCamPos));
    FragColor = vec4(vColor * a * uBeamGain, 1.0);
}
