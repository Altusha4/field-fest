#version 330 core
#include "common.glsl"

in float vT;
in vec3  vColor;
in vec3  vWorld;
in vec3  vNormal;
out vec4 FragColor;

void main() {
    vec3 albedo = vColor * mix(0.3, 1.0, vT);
    FragColor = vec4(shade(albedo, normalize(vNormal), vWorld, vT), 1.0);
}
