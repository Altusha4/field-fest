#pragma once
#include "Audio.h"
#include "Camera.h"
#include "FrameUniforms.h"
#include "Game.h"
#include "GlowSprites.h"
#include "Hud.h"
#include "Performers.h"
#include "Show.h"

#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

struct Options {
    int width = 1280, height = 720;
    std::string shotPath;
    int shotFrame = 120;
    float elapsed = -1.0f;
    int view = 0;
    int song = 0;
    int location = 0;
    bool naive = false;
    int crowd = 3000;
    bool bench = false;
    bool autopilot = false;
    bool mute = false;
    bool noHud = false;
};

class App {
public:
    explicit App(const Options& o);
    ~App();
    int run();

private:
    bool init();
    void shutdown();
    void onKey(int key, int action);
    void onCursor(double x, double y);
    void onMouseButton(int button, int action);
    void onScroll(double dy);
    void startShow(double at);
    void selectSong(int i);
    void update(float dt);
    void render();
    void fillFrameUniforms(float aspect);
    void toast(const std::string& m);
    void fillHud(HudData& d) const;
    bool showLive() const;
    float beatsNow() const;
    void benchTick(double frameSeconds);

    Options opt_;
    GLFWwindow* win_ = nullptr;
    int fbW_ = 0, fbH_ = 0;

    struct Gfx;
    std::unique_ptr<Gfx> gfx_;

    Camera cam_;
    Game game_;
    Show show_;
    Audio audio_;
    Hud hud_;
    FrameUniforms fu_;
    music::Levels levels_;
    std::vector<GlowSprites::Sprite> sprites_;

    double songTime_ = 0.0;
    Performers::Cast cast_ = Performers::Cast::None;
    bool instanced_ = true, windOn_ = true, paused_ = false, vsync_ = true, showHelp_ = false, autopilot_ = false;
    bool laneDown_[4] = {false, false, false, false};
    float wind_ = 1.0f, animTime_ = 0.0f;
    int lastCombo_ = 0;
    int crowdCount_ = 3000, crowdDrawCalls_ = 0;
    float sunAz_ = 0.0f, sunAltOffset_ = 0.0f;

    bool dragging_ = false;
    double lastX_ = 0, lastY_ = 0;

    float fps_ = 0, frameMs_ = 0;
    double fpsTime_ = 0;
    int fpsFrames_ = 0, frame_ = 0;
    struct BenchResult {
        int crowd;
        bool instanced;
        double fps, frameMs, crowdGpuMs;
        int drawCalls;
    };
    std::vector<BenchResult> benchResults_;
    int benchStep_ = 0, benchFrames_ = 0;
    double benchTime_ = -2.0, benchFrameSum_ = 0, benchGpuSum_ = 0;
    std::string benchStatus_;
    std::string message_;
    float messageTimer_ = 0;
};
