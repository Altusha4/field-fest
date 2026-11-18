#version 330 core
out vec2 vNdc;

void main() {
    vec2 p[3] = vec2[3](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    vNdc = p[gl_VertexID];
    gl_Position = vec4(vNdc, 0.0, 1.0);
}
