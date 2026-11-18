
uniform vec3  uSunDir;
uniform vec3  uSunColor;
uniform vec3  uAmbient;
uniform vec3  uSkyHorizon;
uniform vec3  uSkyZenith;
uniform float uNight;
uniform float uTime;
uniform vec3  uCamPos;

uniform vec3  uSpotPos[8];
uniform vec3  uSpotDir[8];
uniform vec3  uSpotColor[8];
uniform float uSpotCos;
uniform vec3  uPointPos[10];
uniform vec3  uPointColor[10];

uniform float uKickAge;
uniform float uKickPower;
uniform float uBeats;
uniform float uHype;
uniform vec4  uLevels;

uniform int   uLocation;
uniform float uWaterLevel;

float terrainHeight(vec2 p) {
    float hills = 1.4 * sin(p.x * 0.07) * cos(p.y * 0.06)
                + 0.6 * sin(p.x * 0.19 + p.y * 0.13)
                + 0.25 * cos(p.y * 0.31 - p.x * 0.11);
    float d = length(p - vec2(0.0, 6.0));
    float far = smoothstep(22.0, 70.0, d), bowl = smoothstep(8.0, 45.0, d);
    if (uLocation == 1) {
        float h = 0.45 * hills * far + 1.2 * bowl;
        float shore = max(1.0 - smoothstep(-34.0, -13.0, p.y), 1.0 - smoothstep(-75.0, -42.0, p.x));
        return mix(h, -3.2, shore);
    }
    if (uLocation == 2) return 1.9 * hills * far + 2.2 * bowl;
    return hills * far + 2.2 * bowl;
}

float crowdMask(vec2 p) {
    float halfWidth = 10.0 + 0.7 * (p.y - 3.5);
    return smoothstep(0.0, 2.0, p.y - 3.0) * smoothstep(0.0, 3.0, 41.0 - p.y)
         * smoothstep(0.0, 2.5, halfWidth - abs(p.x));
}

vec3 skyColor(vec3 d) {
    float y = max(d.y, 0.0);
    vec3 c = mix(uSkyHorizon, uSkyZenith, pow(smoothstep(0.0, 0.85, y), 0.55));
    float s = max(dot(d, uSunDir), 0.0);
    c += uSunColor * pow(s, 6.0) * 0.45;
    return c;
}

float fogFactor(vec3 p) { return smoothstep(45.0, 115.0, distance(p, uCamPos)); }

vec3 applyFog(vec3 c, vec3 p) {
    vec3 vd = normalize(p - uCamPos);
    return mix(c, skyColor(normalize(vec3(vd.x, max(vd.y, 0.03), vd.z))), fogFactor(p));
}

vec3 shade(vec3 albedo, vec3 n, vec3 p, float tip) {
    float diff = max(dot(n, uSunDir), 0.0);
    vec3 c = albedo * (uAmbient + uSunColor * diff);

    vec3 viewDir = normalize(p - uCamPos);
    float back = pow(max(dot(viewDir, uSunDir), 0.0), 5.0);
    c += albedo * uSunColor * vec3(1.0, 0.95, 0.5) * back * tip * 2.0;

    vec3 lit = albedo + 0.06;
    vec3 light = vec3(0.0);
    for (int i = 0; i < 8; ++i) {
        vec3 L = uSpotPos[i] - p;
        float d = length(L);
        L /= d;
        float cone = smoothstep(uSpotCos, mix(uSpotCos, 1.0, 0.6), dot(-L, uSpotDir[i]));
        float wrap = max(dot(n, L) * 0.6 + 0.4, 0.0);
        light += uSpotColor[i] * cone * wrap / (1.0 + d * d * 0.0025);
    }
    for (int i = 0; i < 10; ++i) {
        vec3 L = uPointPos[i] - p;
        float d = length(L);
        float wrap = max(dot(n, L / d) * 0.5 + 0.5, 0.0);
        light += uPointColor[i] * wrap / (1.0 + d * d * 0.03);
    }
    c += lit * light / (1.0 + 0.15 * light);
    return applyFog(c, p);
}
