#version 330 core
#include "common.glsl"

in vec2 vNdc;
out vec4 FragColor;

uniform vec3 uCamRight;
uniform vec3 uCamUp;
uniform vec3 uCamForward;
uniform vec2 uTanHalfFov;
uniform float uAurora;

float hash13(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.zyx + 31.32);
    return fract((p.x + p.y) * p.z);
}

void main() {
    vec3 d = normalize(uCamForward + vNdc.x * uTanHalfFov.x * uCamRight + vNdc.y * uTanHalfFov.y * uCamUp);
    vec3 c = skyColor(d);

    float s = max(dot(d, uSunDir), 0.0);
    c += uSunColor * smoothstep(0.9988, 0.9994, s) * 1.5;

    if (uNight > 0.01 && d.y > 0.0) {
        vec3 q = d * 220.0;
        float h = hash13(floor(q));
        if (h > 0.996) {
            float twinkle = 0.6 + 0.4 * sin(uTime * 3.0 + h * 100.0);
            float star = 1.0 - smoothstep(0.0, 0.35, length(fract(q) - 0.5));
            c += vec3(0.9, 0.95, 1.0) * star * twinkle * uNight * smoothstep(0.0, 0.15, d.y);
        }
    }
    if (uAurora > 0.0 && uNight > 0.01 && d.y > 0.02) {
        float az = atan(d.x, -d.z);
        float wave = sin(az * 3.0 + uTime * 0.12 + 0.8 * sin(az * 7.0 - uTime * 0.2));
        float center = 0.34 + 0.07 * wave;
        float x = (d.y - center) / 0.09;
        float band = exp(-x * x) + 0.5 * exp(-((d.y - center - 0.1) / 0.12) * ((d.y - center - 0.1) / 0.12));
        float rays = 0.55 + 0.45 * sin(az * 70.0 + wave * 5.0 + uTime * 0.4);
        vec3 col = mix(vec3(0.15, 1.0, 0.45), vec3(0.65, 0.25, 1.0), smoothstep(center + 0.06, center + 0.26, d.y));
        c += col * band * rays * uAurora * smoothstep(0.0, 0.6, uNight) * 1.1;
    }
    FragColor = vec4(c, 1.0);
}
