#include "FrameUniforms.h"
#include "Location.h"
#include "MathUtil.h"
#include "Shader.h"

#include <cmath>

using mathx::smoothstep;

static glm::vec3 lerp3(const glm::vec3& a, const glm::vec3& b, float t) { return a + (b - a) * t; }

void FrameUniforms::computeLighting(float az, float alt) {
    sunDir = glm::normalize(glm::vec3(std::sin(az) * std::cos(alt), std::sin(alt), -std::cos(az) * std::cos(alt)));
    float day = smoothstep(0.02f, 0.35f, alt);
    float dusk = smoothstep(-0.14f, 0.03f, alt);
    night = 1.0f - dusk;

    auto pick = [&](glm::vec3 nightC, glm::vec3 duskC, glm::vec3 dayC) { return lerp3(nightC, lerp3(duskC, dayC, day), dusk); };
    skyHorizon = pick({0.035f, 0.04f, 0.10f}, {0.98f, 0.46f, 0.24f}, {1.0f, 0.66f, 0.40f});
    skyZenith  = pick({0.008f, 0.01f, 0.035f}, {0.13f, 0.09f, 0.28f}, {0.22f, 0.28f, 0.55f});
    sunColor   = pick({0.0f, 0.0f, 0.0f}, {1.12f, 0.58f, 0.30f}, {1.15f, 0.92f, 0.70f});
    ambient    = pick({0.035f, 0.045f, 0.09f}, {0.19f, 0.16f, 0.28f}, {0.24f, 0.24f, 0.34f});
    const location::Location& l = location::cur();
    skyHorizon *= l.horizonTint;
    skyZenith *= l.zenithTint;
    sunColor *= l.sunTint;
    ambient *= l.ambientTint;
    if (l.snow) ambient += glm::vec3(0.07f, 0.09f, 0.16f) * night;
}

void FrameUniforms::apply(Shader& s) const {
    s.set("uViewProj", viewProj);
    s.set("uCamPos", camPos);
    s.set("uCamRight", camRight);
    s.set("uCamUp", camUp);
    s.set("uCamForward", camForward);
    s.set("uTanHalfFov", tanHalfFov);
    s.set("uSunDir", sunDir);
    s.set("uSunColor", sunColor);
    s.set("uAmbient", ambient);
    s.set("uSkyHorizon", skyHorizon);
    s.set("uSkyZenith", skyZenith);
    s.set("uNight", night);
    s.set("uTime", time);
    s.setVec3Array("uSpotPos", spotPos.data(), kSpots);
    s.setVec3Array("uSpotDir", spotDir.data(), kSpots);
    s.setVec3Array("uSpotColor", spotColor.data(), kSpots);
    s.set("uSpotCos", spotCos);
    s.setVec3Array("uPointPos", pointPos.data(), kPoints);
    s.setVec3Array("uPointColor", pointColor.data(), kPoints);
    s.set("uKickAge", kickAge);
    s.set("uKickPower", kickPower);
    s.set("uBeats", beats);
    s.set("uHype", hype);
    s.set("uLevels", levels);
    location::apply(s);
}
