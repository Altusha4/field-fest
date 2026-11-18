#include "App.h"

#include "Beams.h"
#include "Config.h"
#include "Crowd.h"
#include "Location.h"
#include "MathUtil.h"
#include "Props.h"
#include "Water.h"
#include "Performers.h"
#include "GpuTimer.h"
#include "Grass.h"
#include "Screenshot.h"
#include "Shader.h"
#include "Sky.h"
#include "Stage.h"
#include "Terrain.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

#ifndef SONGS_DIR
#define SONGS_DIR "songs"
#endif

struct App::Gfx {
    Shader skyShader, groundShader, grassShader, glowShader, meshShader, crowdShader, beamShader, waterShader, propsShader;
    Sky sky;
    Terrain terrain;
    Grass grass;
    Stage stage;
    Crowd crowd;
    Performers performers;
    Water water;
    Props props;
    Beams beams;
    GlowSprites glow;
    GpuTimer crowdTimer;

    Shader* all[9] = {&skyShader, &groundShader, &grassShader, &glowShader, &meshShader, &crowdShader, &beamShader, &waterShader, &propsShader};
    static constexpr const char* kNames[9] = {"sky", "ground", "grass", "glow", "mesh", "crowd", "beam", "water", "props"};

    bool loadShaders() {
        bool ok = true;
        for (size_t i = 0; i < std::size(all); ++i) ok &= all[i]->load(std::string(kNames[i]) + ".vert", std::string(kNames[i]) + ".frag");
        return ok;
    }
    bool reloadShaders() {
        bool ok = true;
        for (Shader* s : all) ok &= s->reload();
        return ok;
    }
    ~Gfx() {
        sky.release();
        terrain.release();
        grass.release();
        stage.release();
        crowd.release();
        performers.release();
        water.release();
        props.release();
        beams.release();
        glow.release();
        crowdTimer.release();
    }
};

App::App(const Options& o) : opt_(o) {}
App::~App() { shutdown(); }

static void glfwError(int code, const char* msg) { std::cerr << "[glfw] error " << code << ": " << msg << "\n"; }

bool App::init() {
    glfwSetErrorCallback(glfwError);
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    std::string title = std::string(cfg::kTopic) + "  |  " + cfg::kTeam;
    win_ = glfwCreateWindow(opt_.width, opt_.height, title.c_str(), nullptr, nullptr);
    if (!win_) {
        std::cerr << "[app] could not create an OpenGL 3.3 core window\n";
        return false;
    }
    glfwMakeContextCurrent(win_);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "[app] failed to load OpenGL functions (GLAD)\n";
        return false;
    }
    std::cout << "[app] OpenGL " << glGetString(GL_VERSION) << " on " << glGetString(GL_RENDERER) << "\n";
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(win_, this);
    glfwSetKeyCallback(win_, [](GLFWwindow* w, int key, int, int action, int) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onKey(key, action);
    });
    glfwSetCursorPosCallback(win_, [](GLFWwindow* w, double x, double y) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onCursor(x, y);
    });
    glfwSetMouseButtonCallback(win_, [](GLFWwindow* w, int b, int action, int) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onMouseButton(b, action);
    });
    glfwSetScrollCallback(win_, [](GLFWwindow* w, double, double dy) {
        static_cast<App*>(glfwGetWindowUserPointer(w))->onScroll(dy);
    });
    if (!hud_.init(win_)) {
        std::cerr << "[app] ImGui init failed\n";
        return false;
    }

    gfx_ = std::make_unique<Gfx>();
    if (!gfx_->loadShaders()) {
        std::cerr << "[app] shader loading failed - see errors above\n";
        return false;
    }
    gfx_->sky.init();
    if (!gfx_->terrain.init()) return false;
    if (!gfx_->grass.init(cfg::kGrassBlades, cfg::kFieldSize)) return false;
    gfx_->stage.init();
    gfx_->crowd.init();
    gfx_->performers.init();
    gfx_->water.init();
    gfx_->props.init();
    gfx_->beams.init(32);
    gfx_->glow.init(12000);
    gfx_->crowdTimer.init();

    show_.init(gfx_->crowd.phones());
    for (const char* dir : {SONGS_DIR, "songs", "../songs"}) {
        int n = music::loadTracks(dir);
        if (n > 0) {
            std::cout << "[app] " << n << " song(s) loaded from " << dir << "\n";
            break;
        }
    }
    if (opt_.shotPath.empty() && !opt_.bench) audio_.init();
    audio_.setMuted(opt_.mute);

    crowdCount_ = std::clamp(opt_.crowd, 1, gfx_->crowd.maxCount());
    instanced_ = !opt_.naive;
    cam_.setPreset(opt_.view, true);
    autopilot_ = opt_.autopilot;
    location::set(opt_.location);
    selectSong(opt_.song);
    if (opt_.bench) {
        vsync_ = false;
        glfwSwapInterval(0);
        startShow(40.0);
        paused_ = true;
    } else if (opt_.elapsed >= 0.0f) {
        startShow(opt_.elapsed);
    }
    toast("LEFT / RIGHT = choose a song, ENTER = start.  F1 = help");
    return true;
}

void App::shutdown() {
    if (!win_) return;
    audio_.shutdown();
    gfx_.reset();
    hud_.shutdown();
    glfwDestroyWindow(win_);
    glfwTerminate();
    win_ = nullptr;
}

void App::toast(const std::string& m) {
    message_ = m;
    messageTimer_ = 3.0f;
    std::cout << "[app] " << m << "\n";
}

void App::startShow(double at) {
    game_.reset();
    game_.start();
    if (at > 0.0) game_.skipTo(at);
    show_.reset();
    songTime_ = at;
    lastCombo_ = 0;
    paused_ = false;
    audio_.seek(at);
    audio_.play();
}

void App::selectSong(int i) {
    if (game_.state() == Game::State::Playing) return;
    audio_.pause();
    music::setSong(i);
    int s = music::currentSong();
    game_.reloadChart();
    game_.loadBest(s == 0 ? "best_score.txt" : "best_score_" + std::to_string(s + 1) + ".txt");
    show_.reset();
    songTime_ = 0.0;
    const music::Song& song = music::song(s);
    std::string title = song.title;
    if (song.track < 0) cast_ = Performers::Cast::None;
    else if (title.find("Nurtas") != std::string::npos || title.find("Tun") != std::string::npos) cast_ = Performers::Cast::Singer;
    else cast_ = Performers::Cast::Duo;
    std::cout << "[app] song: " << song.title << " (" << song.bpm << " BPM, " << game_.notes().size() << " notes)\n";
}

static const int kLaneKeys[4][2] = {{GLFW_KEY_LEFT, GLFW_KEY_D}, {GLFW_KEY_DOWN, GLFW_KEY_F}, {GLFW_KEY_UP, GLFW_KEY_J}, {GLFW_KEY_RIGHT, GLFW_KEY_K}};

static int laneOfKey(int key, bool letters) {
    for (int lane = 0; lane < 4; ++lane)
        if (key == kLaneKeys[lane][0] || (letters && key == kLaneKeys[lane][1])) return lane;
    return -1;
}

bool App::showLive() const { return game_.state() == Game::State::Playing || game_.state() == Game::State::Won; }
float App::beatsNow() const { return showLive() ? levels_.beats : animTime_ * 0.8f; }

void App::onKey(int key, int action) {
    if (action != GLFW_PRESS) return;
    if (game_.state() != Game::State::Playing && (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)) {
        selectSong(music::currentSong() + (key == GLFW_KEY_RIGHT ? 1 : -1));
        return;
    }
    if (game_.state() != Game::State::Playing && (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN)) {
        location::set(location::current() + (key == GLFW_KEY_DOWN ? 1 : -1));
        if (game_.state() != Game::State::Ready) selectSong(music::currentSong());
        toast(std::string("Location: ") + location::cur().name);
        return;
    }
    int lane = laneOfKey(key, cam_.mode == Camera::Mode::Orbit);
    if (lane >= 0) {
        game_.press(lane, songTime_);
        return;
    }
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(win_, 1); break;
        case GLFW_KEY_ENTER:
        case GLFW_KEY_KP_ENTER:
            if (game_.state() != Game::State::Playing) startShow(0.0);
            break;
        case GLFW_KEY_R:
            game_.reset();
            show_.reset();
            audio_.pause();
            songTime_ = 0.0;
            sunAltOffset_ = 0.0f;
            toast("Show reset");
            break;
        case GLFW_KEY_I:
            instanced_ = !instanced_;
            toast(instanced_ ? "Crowd instancing ON: 1 draw call for everybody" : "Crowd instancing OFF: 1 draw call per person");
            break;
        case GLFW_KEY_T:
            windOn_ = !windOn_;
            toast(windOn_ ? "Wind ON" : "Wind OFF (blades in rest pose)");
            break;
        case GLFW_KEY_P:
            paused_ = !paused_;
            if (game_.state() == Game::State::Playing) paused_ ? audio_.pause() : audio_.play();
            break;
        case GLFW_KEY_M:
            audio_.setMuted(!audio_.muted());
            toast(audio_.muted() ? "Music muted" : "Music on");
            break;
        case GLFW_KEY_O:
            autopilot_ = !autopilot_;
            toast(autopilot_ ? "Autopilot ON (demo): perfect play" : "Autopilot OFF");
            break;
        case GLFW_KEY_C:
            cam_.toggleMode();
            toast(cam_.mode == Camera::Mode::FreeFly ? "Free-fly: WASD, Space/Ctrl, drag" : "Orbit camera");
            break;
        case GLFW_KEY_TAB:
            cam_.setPreset(cam_.preset() + 1);
            toast(std::string("Camera: ") + Camera::presetName(cam_.preset()));
            break;
        case GLFW_KEY_V:
            vsync_ = !vsync_;
            glfwSwapInterval(vsync_ ? 1 : 0);
            break;
        case GLFW_KEY_F1:
        case GLFW_KEY_H: showHelp_ = !showHelp_; break;
        case GLFW_KEY_1:
        case GLFW_KEY_2:
        case GLFW_KEY_3:
        case GLFW_KEY_4: {
            const int sizes[4] = {1000, 3000, 20000, 50000};
            crowdCount_ = std::min(sizes[key - GLFW_KEY_1], gfx_->crowd.maxCount());
            toast("Crowd: " + std::to_string(crowdCount_) + " people");
            break;
        }
        case GLFW_KEY_F5: toast(gfx_->reloadShaders() ? "Shaders reloaded" : "Shader error - see console"); break;
        case GLFW_KEY_F12: {
            std::filesystem::create_directories("screenshots");
            std::string name = "screenshots/shot_" + std::to_string((long long)std::time(nullptr)) + ".bmp";
            render();
            saveScreenshotBMP(name, fbW_, fbH_);
            toast("Saved " + name);
            break;
        }
        default: break;
    }
}

void App::onCursor(double x, double y) {
    if (dragging_) cam_.rotate((float)(x - lastX_), (float)(y - lastY_));
    lastX_ = x;
    lastY_ = y;
}

void App::onMouseButton(int button, int action) {
    if (button != GLFW_MOUSE_BUTTON_LEFT && button != GLFW_MOUSE_BUTTON_RIGHT) return;
    if (action == GLFW_PRESS && ImGui::GetIO().WantCaptureMouse) return;
    dragging_ = action == GLFW_PRESS;
    glfwGetCursorPos(win_, &lastX_, &lastY_);
}

void App::onScroll(double dy) {
    if (!ImGui::GetIO().WantCaptureMouse) cam_.zoom((float)dy);
}

void App::update(float dt) {
    auto down = [this](int k) { return glfwGetKey(win_, k) == GLFW_PRESS; };
    bool fast = down(GLFW_KEY_LEFT_SHIFT) || down(GLFW_KEY_RIGHT_SHIFT);
    bool fly = cam_.mode == Camera::Mode::FreeFly;
    float fwd = fly ? (down(GLFW_KEY_W) ? 1.f : 0.f) - (down(GLFW_KEY_S) ? 1.f : 0.f) : 0.f;
    float rgt = fly ? (down(GLFW_KEY_D) ? 1.f : 0.f) - (down(GLFW_KEY_A) ? 1.f : 0.f) : 0.f;
    float upk = fly ? (down(GLFW_KEY_SPACE) ? 1.f : 0.f) - (down(GLFW_KEY_LEFT_CONTROL) ? 1.f : 0.f) : 0.f;
    for (int i = 0; i < 4; ++i) laneDown_[i] = down(kLaneKeys[i][0]) || (!fly && down(kLaneKeys[i][1]));

    if (down(GLFW_KEY_Q)) sunAz_ -= dt * 0.8f;
    if (down(GLFW_KEY_E)) sunAz_ += dt * 0.8f;
    if (down(GLFW_KEY_Z)) sunAltOffset_ -= dt * 0.25f;
    if (down(GLFW_KEY_X)) sunAltOffset_ += dt * 0.25f;
    sunAltOffset_ = std::clamp(sunAltOffset_, -0.6f, 1.0f);

    float simDt = paused_ ? 0.0f : dt;
    animTime_ += simDt;
    wind_ += ((windOn_ ? 1.0f : 0.0f) - wind_) * std::min(1.0f, dt * 3.0f);

    Game::State before = game_.state();
    if (before == Game::State::Playing && !paused_) {
        songTime_ += dt;
        if (audio_.ok() && audio_.playing()) {
            double err = audio_.time() - songTime_;
            if (std::fabs(err) > 0.08) songTime_ = audio_.time();
            else songTime_ += err * 0.15;
        }
        if (autopilot_) game_.autoplay(songTime_);
        game_.update(songTime_);
    } else if (before == Game::State::Won) {
        songTime_ += simDt;
    }
    Game::State after = game_.state();
    if (after != before) {
        if (after == Game::State::Won) {
            show_.celebrate();
            toast("ENCORE! The crowd loved it.");
        } else if (after == Game::State::Lost) {
            audio_.pause();
            toast("The crowd went home... press ENTER to try again");
        }
    }

    for (const Game::Event& e : game_.takeEvents()) {
        if (e.judge == Game::Judge::Miss) show_.miss();
        else if (e.judge == Game::Judge::Perfect) show_.trigger(e.lane, 1.0f);
        else if (e.judge == Game::Judge::Good) show_.trigger(e.lane, 0.75f);
        else show_.trigger(e.lane, 0.8f);
    }
    if (game_.combo() / 25 > lastCombo_ / 25) show_.firework();
    lastCombo_ = game_.combo();

    const bool live = showLive();
    levels_ = live ? music::levels(songTime_) : music::Levels{};
    show_.update(simDt, animTime_, songTime_, levels_, game_.hype(), live);
    gfx_->performers.update(animTime_, beatsNow(), live ? show_.jump() / 0.4f : 0.1f, cast_);
    cam_.update(glm::vec3(rgt, upk, fwd), fast ? 25.0f : 8.0f, dt, animTime_);
    messageTimer_ = std::max(0.0f, messageTimer_ - dt);
}

void App::fillFrameUniforms(float aspect) {
    fu_.viewProj = cam_.projection(aspect) * cam_.view();
    fu_.camPos = cam_.eye();
    fu_.camForward = cam_.forward();
    fu_.camRight = cam_.right();
    fu_.camUp = cam_.up();
    fu_.tanHalfFov = glm::vec2(std::tan(cam_.fovY * 0.5f) * aspect, std::tan(cam_.fovY * 0.5f));
    float k = game_.state() == Game::State::Ready ? 0.0f : mathx::smoothstep(0.0f, 40.0f, (float)songTime_);
    fu_.computeLighting(sunAz_, 0.22f + (-0.16f - 0.22f) * k + sunAltOffset_);
    fu_.time = animTime_;
    fu_.beats = beatsNow();
    fu_.kickAge = levels_.kickAge;
    fu_.kickPower = show_.kickPower();
    fu_.hype = game_.hype();
    fu_.levels = glm::vec4(levels_.kick, levels_.snare, levels_.bass, levels_.lead);
    show_.fillUniforms(fu_);
    const auto targets = gfx_->performers.spotTargets();
    for (int i = 0; i < 2; ++i) {
        fu_.pointPos[8 + i] = targets[i];
        fu_.pointColor[8 + i] = i < gfx_->performers.spotCount() ? glm::vec3(1.0f, 0.88f, 0.72f) * (0.4f + 0.5f * fu_.night) : glm::vec3(0.0f);
    }
    fu_.ambient += show_.accent() * 0.07f * show_.showOn();
}

void App::render() {
    if (fbW_ <= 0 || fbH_ <= 0) return;
    Gfx& g = *gfx_;
    fillFrameUniforms((float)fbW_ / (float)fbH_);

    auto bind = [this](Shader& s) {
        s.use();
        fu_.apply(s);
    };

    glViewport(0, 0, fbW_, fbH_);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    bind(g.skyShader);
    g.sky.draw(g.skyShader);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    bind(g.groundShader);
    g.terrain.draw(g.groundShader, cam_.eye());
    if (location::cur().water) {
        bind(g.waterShader);
        g.water.draw(g.waterShader, cam_.eye());
    }

    bind(g.propsShader);
    g.props.draw(g.propsShader, location::current());

    bind(g.meshShader);
    g.stage.draw(g.meshShader, show_.accent(), show_.showOn());
    g.performers.draw(g.meshShader);

    bind(g.crowdShader);
    g.crowdTimer.begin();
    crowdDrawCalls_ = g.crowd.draw(g.crowdShader, crowdCount_, instanced_, show_.jump(), show_.arms(), show_.sway());
    g.crowdTimer.end();

    glDisable(GL_CULL_FACE);
    bind(g.grassShader);
    g.grass.draw(g.grassShader, cam_.eye(), wind_);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    bind(g.beamShader);
    g.beams.draw(g.beamShader, show_.beams(), 0.2f + 0.8f * fu_.night);

    show_.fillSprites(sprites_, cam_.focus(), fu_.night);
    bind(g.glowShader);
    g.glow.draw(g.glowShader, sprites_);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        static int reported = 0;
        if (reported++ < 5) std::cerr << "[gl] error 0x" << std::hex << err << std::dec << " during frame\n";
    }
}

void App::fillHud(HudData& d) const {
    d.fps = fps_;
    d.frameMs = frameMs_;
    d.crowd = crowdCount_;
    d.crowdDrawCalls = crowdDrawCalls_;
    d.gpuCrowdMs = (float)gfx_->crowdTimer.milliseconds();
    d.instanced = instanced_;
    d.wind = windOn_;
    d.paused = paused_;
    d.vsync = vsync_;
    d.freeCam = cam_.mode == Camera::Mode::FreeFly;
    d.camName = Camera::presetName(cam_.preset());
    d.showHelp = showHelp_;
    d.muted = audio_.muted();
    d.audioOk = audio_.ok();
    d.autopilot = autopilot_;
    d.state = game_.state();
    d.score = game_.score();
    d.best = game_.best();
    d.newRecord = game_.newRecord();
    d.combo = game_.combo();
    d.maxCombo = game_.maxCombo();
    d.perfect = game_.perfect();
    d.good = game_.good();
    d.miss = game_.miss();
    d.hype = game_.hype();
    d.accuracy = game_.accuracy();
    d.grade = game_.grade();
    d.judge = game_.lastJudge();
    d.judgeAge = (float)(songTime_ - game_.lastJudgeTime());
    d.songTime = songTime_;
    d.section = music::sectionName(levels_.section);
    d.notes = &game_.notes();
    d.noteStates = &game_.noteStates();
    d.firstNote = game_.firstPending();
    for (int i = 0; i < 4; ++i) {
        d.laneDown[i] = laneDown_[i];
        d.laneFlash[i] = show_.laneFlash(i);
    }
    d.sunAzDeg = std::fmod(glm::degrees(sunAz_) + 3600.0f, 360.0f);
    d.bench = benchStatus_;
    d.message = message_;
    d.messageAlpha = std::min(1.0f, messageTimer_);
}

static const int kBenchCrowd[] = {1000, 3000, 10000, 20000, 50000};
static const int kBenchSteps = 10;

void App::benchTick(double frameSeconds) {
    if (benchStep_ >= kBenchSteps) return;
    int crowd = std::min(kBenchCrowd[benchStep_ / 2], gfx_->crowd.maxCount());
    bool inst = benchStep_ % 2 == 0;
    crowdCount_ = crowd;
    instanced_ = inst;
    benchTime_ += frameSeconds;
    if (benchTime_ > 1.5) {
        benchFrameSum_ += frameSeconds;
        benchGpuSum_ += gfx_->crowdTimer.milliseconds();
        ++benchFrames_;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "BENCHMARK %d/%d: %d people, instancing %s", benchStep_ + 1, kBenchSteps, crowd, inst ? "ON" : "OFF");
    benchStatus_ = buf;
    if (benchTime_ < 4.5) return;
    double ms = 1000.0 * benchFrameSum_ / std::max(1, benchFrames_);
    benchResults_.push_back({crowd, inst, 1000.0 / ms, ms, benchGpuSum_ / std::max(1, benchFrames_), crowdDrawCalls_});
    benchTime_ = benchFrameSum_ = benchGpuSum_ = 0.0;
    benchFrames_ = 0;
    if (++benchStep_ < kBenchSteps) return;

    std::ofstream csv("benchmark.csv");
    csv << "people,instancing,draw_calls,fps,frame_ms,crowd_gpu_ms\n";
    std::cout << "\n| People | Instancing | Draw calls (crowd) | FPS | Frame time, ms | Crowd GPU time, ms |\n"
              << "|---|---|---|---|---|---|\n";
    for (const auto& r : benchResults_) {
        csv << r.crowd << "," << (r.instanced ? "on" : "off") << "," << r.drawCalls << "," << r.fps << "," << r.frameMs << ","
            << r.crowdGpuMs << "\n";
        snprintf(buf, sizeof(buf), "| %d | %s | %d | %.0f | %.2f | %.2f |\n", r.crowd, r.instanced ? "ON" : "OFF", r.drawCalls, r.fps,
                 r.frameMs, r.crowdGpuMs);
        std::cout << buf;
    }
    std::cout << "[bench] saved benchmark.csv (" << location::cur().name << ", " << fbW_ << "x" << fbH_ << ", "
              << glGetString(GL_RENDERER) << ")\n";
    glfwSetWindowShouldClose(win_, 1);
}

int App::run() {
    if (!init()) {
        std::cerr << "[app] initialization failed\n";
        return 1;
    }
    glEnable(GL_MULTISAMPLE);
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(win_)) {
        glfwPollEvents();
        double now = glfwGetTime();
        double raw = now - last;
        float dt = (float)std::min(0.05, raw);
        last = now;

        fpsTime_ += raw;
        ++fpsFrames_;
        if (fpsTime_ >= 0.5) {
            fps_ = (float)(fpsFrames_ / fpsTime_);
            frameMs_ = (float)(1000.0 * fpsTime_ / fpsFrames_);
            fpsTime_ = 0;
            fpsFrames_ = 0;
        }

        glfwGetFramebufferSize(win_, &fbW_, &fbH_);
        if (opt_.bench) benchTick(raw);
        update(dt);
        render();

        bool capture = !opt_.shotPath.empty() && frame_ == opt_.shotFrame;
        if (!opt_.noHud) {
            hud_.beginFrame();
            HudData d;
            fillHud(d);
            hud_.draw(d);
            hud_.endFrame();
        }

        if (capture) {
            saveScreenshotBMP(opt_.shotPath, fbW_, fbH_);
            glfwSetWindowShouldClose(win_, 1);
        }
        glfwSwapBuffers(win_);
        ++frame_;
    }
    return 0;
}
