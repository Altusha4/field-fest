#include "Location.h"
#include "MathUtil.h"
#include "Shader.h"

namespace location {
namespace {
const Location kList[Count] = {
    {"Meadow", "tall grass at sunset",
     {0.0f, 0.0f, 0.0f}, 0.0f,
     {0.10f, 0.28f, 0.05f}, {0.42f, 0.62f, 0.12f}, {0.72f, 0.60f, 0.22f}, 1.0f, 1.0f,
     {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, {1, 1, 1}, false, false, 0.0f},
    {"Beach", "dunes and the sea",
     {0.9f, 0.79f, 0.58f}, 1.0f,
     {0.36f, 0.38f, 0.16f}, {0.78f, 0.74f, 0.44f}, {0.86f, 0.78f, 0.52f}, 1.25f, 0.28f,
     {1.05f, 0.97f, 0.95f}, {0.92f, 1.03f, 1.15f}, {1, 1, 1}, {1.0f, 1.0f, 1.08f}, true, false, 0.0f},
    {"Winter", "snow and northern lights",
     {0.86f, 0.9f, 0.98f}, 1.0f,
     {0.42f, 0.48f, 0.46f}, {0.84f, 0.9f, 0.95f}, {0.9f, 0.92f, 0.95f}, 0.6f, 0.35f,
     {0.72f, 0.82f, 1.15f}, {0.78f, 0.9f, 1.2f}, {1.0f, 0.9f, 0.95f}, {0.85f, 0.95f, 1.3f}, false, true, 1.0f},
};
int gCurrent = Meadow;
}

void set(int id) { gCurrent = mathx::wrap(id, Count); }
int current() { return gCurrent; }
const Location& get(int id) { return kList[mathx::wrap(id, Count)]; }

void apply(Shader& s) {
    const Location& l = cur();
    s.set("uLocation", gCurrent);
    s.set("uGroundColor", l.groundColor);
    s.set("uGroundRecolor", l.groundRecolor);
    s.set("uGrassDark", l.grassDark);
    s.set("uGrassLight", l.grassLight);
    s.set("uGrassDry", l.grassDry);
    s.set("uGrassHeight", l.grassHeight);
    s.set("uGrassCoverage", l.grassCoverage);
    s.set("uAurora", l.aurora);
    s.set("uWaterLevel", kWaterLevel);
}

}
