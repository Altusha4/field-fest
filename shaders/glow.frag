#version 330 core
in vec2 vQuad;
in vec4 vColor;
out vec4 FragColor;

void main() {
    float d = dot(vQuad, vQuad);
    if (d > 1.0) discard;
    float core = exp(-d * 16.0);
    float halo = exp(-d * 3.5) * 0.35;
    vec3 c = (vColor.rgb * (core * 1.6 + halo) + vec3(core * core * 0.9)) * vColor.a;
    FragColor = vec4(c, 1.0);
}
