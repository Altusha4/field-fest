#version 330 core
#include "common.glsl"

in vec3 vWorld;
in vec3 vNormal;
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(shade(vColor, normalize(vNormal), vWorld, 0.0), 1.0);
}
