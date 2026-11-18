#version 330 core
layout(location = 0) in vec4 aPosSize;
layout(location = 1) in vec4 aColor;

uniform mat4 uViewProj;
uniform vec3 uCamRight;
uniform vec3 uCamUp;

out vec2 vQuad;
out vec4 vColor;

void main() {
    vec2 corners[6] = vec2[6](vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, -1), vec2(1, 1), vec2(-1, 1));
    vec2 c = corners[gl_VertexID];
    vQuad  = c;
    vColor = aColor;
    vec3 world = aPosSize.xyz + (uCamRight * c.x + uCamUp * c.y) * aPosSize.w;
    gl_Position = uViewProj * vec4(world, 1.0);
}
