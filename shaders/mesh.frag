#version 330 core
#include "common.glsl"

in vec3 vWorld;
in vec3 vNormal;
in vec4 vColor;
out vec4 FragColor;

uniform vec4  uScreenRect;
uniform vec3  uAccent;
uniform float uShowOn;

float hash11(float x) { return fract(sin(x * 127.1) * 43758.5453); }

vec3 ledScreen(vec2 uv) {
    vec2 grid = vec2(128.0, 48.0);
    vec2 px = uv * grid;
    float dotMask = 1.0 - smoothstep(0.25, 0.5, length(fract(px) - 0.5));
    vec2 cell = (floor(px) + 0.5) / grid;

    float col = floor(cell.x * 32.0);
    float rnd = hash11(col + floor(uBeats * 2.0) * 7.0);
    float music = 0.4 * uLevels.x + 0.25 * uLevels.z + 0.2 * uLevels.w + 0.15 * uLevels.y;
    float level = (0.1 + 0.9 * music * (0.45 + rnd)) * (0.5 + 0.5 * uHype);
    float bar = step(cell.y, level) * step(0.18, fract(cell.x * 32.0));
    vec3 barCol = mix(uAccent, vec3(1.0), smoothstep(0.5, 1.0, cell.y / max(level, 0.01)) * 0.7);

    float rings = 0.5 + 0.5 * sin(length((cell - 0.5) * vec2(2.6, 1.0)) * 28.0 - uBeats * 6.2831);
    vec3 bg = uAccent * 0.12 * rings;
    vec3 show = bg + barCol * bar * 1.4;
    vec3 idle = uAccent * 0.08 * (0.5 + 0.5 * sin(uv.x * 6.0 + uTime));
    return mix(idle, show, uShowOn) * dotMask;
}

void main() {
    float mat = vColor.a;
    vec3 c;
    if (mat < 0.5) {
        c = shade(vColor.rgb, normalize(vNormal), vWorld, 0.0);
    } else if (mat < 1.5) {
        vec2 uv = (vWorld.xy - uScreenRect.xy) / (uScreenRect.zw - uScreenRect.xy);
        c = applyFog(ledScreen(uv) * 1.6, vWorld);
    } else {
        c = applyFog(uAccent * (0.3 + uShowOn * (0.6 + 1.4 * uLevels.x)), vWorld);
    }
    FragColor = vec4(c, 1.0);
}
