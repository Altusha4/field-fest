#version 330 core
#include "common.glsl"

in vec3 vWorld;
in vec3 vNormal;
out vec4 FragColor;

void main() {
    vec3 n = normalize(vNormal);
    vec3 v = normalize(uCamPos - vWorld);
    vec3 r = reflect(-v, n);
    float fres = 0.02 + 0.98 * pow(1.0 - max(dot(n, v), 0.0), 5.0);
    vec3 deep = vec3(0.0, 0.32, 0.36) * (uAmbient * 2.0 + uSunColor * 0.45);
    vec3 refl = skyColor(normalize(vec3(r.x, max(r.y, 0.02), r.z)));
    vec3 c = mix(deep, refl, fres * 0.85);
    float shallow = 1.0 - smoothstep(0.0, 1.2, vWorld.y - terrainHeight(vWorld.xz));
    float crest = smoothstep(0.12, 0.22, vWorld.y - uWaterLevel);
    float foam = clamp(shallow * (0.6 + 0.4 * sin(uTime * 1.5 + dot(vWorld.xz, vec2(0.3, 0.5)))) + crest * 0.35, 0.0, 1.0);
    c = mix(c, vec3(0.9, 0.95, 1.0) * (uAmbient * 2.5 + uSunColor * 0.6), foam * 0.8);
    c += uSunColor * pow(max(dot(r, uSunDir), 0.0), 300.0) * 4.0;
    for (int i = 0; i < 8; ++i) {
        vec3 L = normalize(uSpotPos[i] - vWorld);
        c += uSpotColor[i] * pow(max(dot(r, L), 0.0), 120.0) * 0.6;
    }
    for (int i = 0; i < 10; ++i) {
        vec3 L = normalize(uPointPos[i] - vWorld);
        c += uPointColor[i] * pow(max(dot(r, L), 0.0), 60.0) * 0.25;
    }
    FragColor = vec4(applyFog(c, vWorld), 1.0);
}
