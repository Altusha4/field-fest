#pragma once
#include "Game.h"
#include "Music.h"

#include <cstdint>
#include <string>
#include <vector>

struct GLFWwindow;

struct HudData {
    float fps = 0, frameMs = 0;
    int crowd = 0, crowdDrawCalls = 0;
    float gpuCrowdMs = 0;
    std::string bench;
    bool instanced = true, wind = true, paused = false, vsync = true, freeCam = false, showHelp = false;
    bool muted = false, audioOk = false, autopilot = false;
    const char* camName = "";

    Game::State state = Game::State::Ready;
    int score = 0, best = 0, combo = 0, maxCombo = 0, perfect = 0, good = 0, miss = 0;
    float hype = 0.5f, accuracy = 1.0f;
    char grade = 'S';
    bool newRecord = false;
    Game::Judge judge = Game::Judge::None;
    float judgeAge = 99.0f;
    double songTime = 0;
    const char* section = "";
    const std::vector<music::Note>* notes = nullptr;
    const std::vector<uint8_t>* noteStates = nullptr;
    int firstNote = 0;
    bool laneDown[4] = {};
    float laneFlash[4] = {};

    float sunAzDeg = 0;
    std::string message;
    float messageAlpha = 0;
};

class Hud {
public:
    bool init(GLFWwindow* window);
    void draw(const HudData& d);
    void beginFrame();
    void endFrame();
    void shutdown();
};
