#version 330 core
layout(location = 0) in vec3 aCone;
layout(location = 1) in vec4 aI0;
layout(location = 2) in vec4 aI1;
layout(location = 3) in vec4 aI2;

uniform mat4 uViewProj;

out float vT;
out vec3  vColor;
out vec3  vWorld;
out vec3  vNormal;

void main() {
    vec3 d  = normalize(aI1.xyz);
    vec3 up = abs(d.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 r  = normalize(cross(d, up));
    vec3 u  = cross(r, d);
    vec3 radial = r * aCone.x + u * aCone.y;
    float radius = mix(aI2.w, aI1.w, aCone.z);
    vWorld  = aI0.xyz + d * aI0.w * aCone.z + radial * radius;
    vNormal = radial;
    vT      = aCone.z;
    vColor  = aI2.rgb;
    gl_Position = uViewProj * vec4(vWorld, 1.0);
}
