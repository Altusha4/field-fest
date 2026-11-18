#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

uniform mat4 uViewProj;

out vec3 vWorld;
out vec3 vNormal;
out vec4 vColor;

void main() {
    vWorld = aPos;
    vNormal = aNormal;
    vColor = aColor;
    gl_Position = uViewProj * vec4(aPos, 1.0);
}
