#include "Hud.h"
#include "Config.h"
#include "Location.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>

namespace {
const ImVec4 kGold(1.0f, 0.78f, 0.35f, 1.0f);
const ImVec4 kOn(0.45f, 0.95f, 0.45f, 1.0f);
const ImVec4 kOff(1.0f, 0.42f, 0.38f, 1.0f);
const ImVec4 kDim(0.75f, 0.75f, 0.82f, 1.0f);

void onOff(const char* label, bool on, const char* onText = "ON", const char* offText = "OFF") {
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::TextColored(on ? kOn : kOff, "%s", on ? onText : offText);
}

void centeredBanner(const char* id, const char* title, const char* subtitle, ImVec4 color) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.40f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin(id, nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing);
    ImGui::SetWindowFontScale(2.0f);
    ImGui::TextColored(color, "%s", title);
    ImGui::SetWindowFontScale(1.15f);
    ImGui::TextUnformatted(subtitle);
    ImGui::End();
}
}

bool Hud::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    float xs = 1.0f, ys = 1.0f;
    glfwGetWindowContentScale(window, &xs, &ys);
    if (xs < 1.0f) xs = 1.0f;
    ImFontConfig fc;
    fc.SizePixels = 15.0f * xs;
    io.Fonts->AddFontDefault(&fc);
    const char* cyrillicFonts[] = {"/System/Library/Fonts/Supplemental/Arial.ttf", "/Library/Fonts/Arial Unicode.ttf",
                                   "C:\\Windows\\Fonts\\arial.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"};
    for (const char* path : cyrillicFonts) {
        if (FILE* f = std::fopen(path, "rb")) {
            std::fclose(f);
            ImFontConfig merge;
            merge.MergeMode = true;
            io.Fonts->AddFontFromFileTTF(path, 15.0f * xs, &merge, io.Fonts->GetGlyphRangesCyrillic());
            break;
        }
    }
    io.FontGlobalScale = 1.0f / xs;
    ImGui::StyleColorsDark();
    ImGuiStyle& st = ImGui::GetStyle();
    st.WindowRounding = 8.0f;
    st.WindowBorderSize = 0.0f;
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) return false;
    return ImGui_ImplOpenGL3_Init("#version 330");
}

void Hud::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Hud::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

namespace {
const ImVec4 kLaneColor[4] = {{1.0f, 0.35f, 0.8f, 1.0f}, {0.3f, 1.0f, 0.4f, 1.0f}, {1.0f, 0.6f, 0.15f, 1.0f}, {0.3f, 0.85f, 1.0f, 1.0f}};
const char* kLaneName[4] = {"LIGHTS", "LASERS", "PYRO", "LIGHTS"};

void arrow(ImDrawList* dl, ImVec2 c, float s, int lane, ImU32 col, bool filled) {
    const float dx[4] = {-1, 0, 0, 1}, dy[4] = {0, 1, -1, 0};
    float x = dx[lane], y = dy[lane];
    ImVec2 tip(c.x + x * s, c.y + y * s);
    ImVec2 l(c.x - x * s * 0.6f - y * s * 0.85f, c.y - y * s * 0.6f + x * s * 0.85f);
    ImVec2 r(c.x - x * s * 0.6f + y * s * 0.85f, c.y - y * s * 0.6f - x * s * 0.85f);
    if (filled) dl->AddTriangleFilled(tip, l, r, col);
    else dl->AddTriangle(tip, l, r, col, 2.5f);
}

ImVec4 withAlpha(ImVec4 c, float a) { return ImVec4(c.x, c.y, c.z, c.w * a); }
}

void Hud::draw(const HudData& d) {
    ImGuiIO& io = ImGui::GetIO();
    const ImGuiWindowFlags panel = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    const bool playing = d.state == Game::State::Playing;

    ImGui::SetNextWindowPos(ImVec2(12, 12));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("info", nullptr, panel);
    ImGui::TextColored(kGold, "%s", cfg::kTopic);
    ImGui::TextColored(kDim, "%s", cfg::kTeam);
    ImGui::TextColored(kDim, "%s", cfg::kGroup);
    ImGui::Separator();
    int now = (int)std::max(0.0, d.songTime), total = (int)music::songEnd();
    const music::Song& song = music::song(music::currentSong());
    ImGui::Text("Location: %s", location::cur().name);
    ImGui::Text("Song: %s", song.title);
    ImGui::SameLine();
    ImGui::TextColored(kDim, " %s, %.0f BPM, %s", song.style, song.bpm, song.difficulty);
    switch (d.state) {
        case Game::State::Ready: ImGui::TextColored(kGold, "Press ENTER to start the show"); break;
        case Game::State::Playing:
            ImGui::Text("%s", d.section);
            ImGui::SameLine();
            ImGui::TextColored(kDim, "  %d:%02d / %d:%02d", now / 60, now % 60, total / 60, total % 60);
            break;
        case Game::State::Won: ImGui::TextColored(kOn, "Encore! Show complete"); break;
        case Game::State::Lost: ImGui::TextColored(kOff, "The crowd went home"); break;
    }
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.6f, 0.6f, 0.7f, 1.0f));
    ImGui::ProgressBar((float)std::clamp(d.songTime / music::songEnd(), 0.0, 1.0), ImVec2(280, 6), "");
    ImGui::PopStyleColor();
    ImGui::Text("Score %d", d.score);
    ImGui::SameLine();
    ImGui::TextColored(kDim, "   Best %d", d.best);
    ImVec4 hypeCol = d.hype < 0.25f ? kOff : d.hype < 0.6f ? kGold : kOn;
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hypeCol);
    ImGui::ProgressBar(d.hype, ImVec2(280, 0), "Crowd hype");
    ImGui::PopStyleColor();
    ImGui::Separator();
    onOff("[I] Crowd instancing:", d.instanced);
    onOff("[T] Wind animation:", d.wind);
    onOff("[P] Animation:", !d.paused, "running", "PAUSED");
    onOff("[M] Music:", d.audioOk && !d.muted, "ON", d.audioOk ? "muted" : "no audio device");
    onOff("[O] Autopilot:", d.autopilot, "ON (demo)", "off");
    ImGui::TextColored(kDim, "[Tab] camera: %s%s", d.freeCam ? "free-fly" : d.camName, d.freeCam ? "" : "   [C] free-fly");
    ImGui::TextColored(kDim, "[F1] / [H] help");
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12, 12), ImGuiCond_Always, ImVec2(1, 0));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("perf", nullptr, panel);
    ImGui::Text("FPS %5.0f   (%.2f ms)", d.fps, d.frameMs);
    ImGui::Text("Crowd: %d people   [1-4]", d.crowd);
    ImGui::Text("Crowd draw calls: %d", d.crowdDrawCalls);
    ImGui::Text("Crowd GPU time: %.2f ms", d.gpuCrowdMs);
    if (!d.bench.empty()) ImGui::TextColored(kGold, "%s", d.bench.c_str());
    onOff("[V] VSync:", d.vsync);
    ImGui::End();

    char sub[512];
    if (d.state == Game::State::Ready) {
        const int cur = music::currentSong();
        const music::Song& s = music::song(cur);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.57f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.6f);
        ImGui::Begin("ready", nullptr, panel | ImGuiWindowFlags_NoInputs);
        ImGui::SetWindowFontScale(1.6f);
        ImGui::TextColored(kGold, "FIELD FEST - you are the light director");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(kDim, "Choose a song with LEFT / RIGHT:");
        for (int i = 0; i < music::songCount(); ++i) {
            const music::Song& o = music::song(i);
            int secs = (int)music::durationOf(i);
            ImVec4 diffCol = o.level == 0 ? kOn : o.level == 1 ? kGold : kOff;
            ImGui::TextColored(i == cur ? ImVec4(1, 1, 1, 1) : kDim, "%s %-14s", i == cur ? ">>" : "  ", o.title);
            ImGui::SameLine();
            ImGui::TextColored(kDim, "%-12s %3.0f BPM  %d:%02d", o.style, o.bpm, secs / 60, secs % 60);
            ImGui::SameLine();
            ImGui::TextColored(diffCol, "%s", o.difficulty);
        }
        ImGui::TextColored(kDim, "Best score for %s: %d", s.title, d.best);
        ImGui::Separator();
        ImGui::TextColored(kDim, "Choose a location with UP / DOWN:");
        for (int i = 0; i < location::Count; ++i) {
            const location::Location& l = location::get(i);
            bool sel = i == location::current();
            ImGui::TextColored(sel ? ImVec4(1, 1, 1, 1) : kDim, "%s %-8s", sel ? ">>" : "  ", l.name);
            ImGui::SameLine();
            ImGui::TextColored(kDim, "%s", l.description);
        }
        ImGui::Separator();
        ImGui::Separator();
        ImGui::TextUnformatted("Notes fall down 4 lanes. Press the arrow key when a note hits the line:\n"
                               "   LEFT / RIGHT = spotlights    DOWN = lasers    UP = pyro (fire)    (or D F J K)\n"
                               "Good timing raises the crowd's HYPE - they jump higher, the show gets wilder.\n"
                               "Misses lower it. If the hype drops to zero, the crowd goes home.\n"
                               "Mouse drag = look around, Tab = change camera, O = autopilot demo.");
        ImGui::Spacing();
        ImGui::SetWindowFontScale(1.25f);
        ImGui::TextColored(kGold, "Press ENTER to start the show");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }
    else if (d.state == Game::State::Won || d.state == Game::State::Lost) {
        snprintf(sub, sizeof(sub),
                 "Score %d   Best %d   %s\n"
                 "Accuracy %d%%   Grade %c   Max combo %d\n"
                 "Perfect %d   Good %d   Miss %d\n\n"
                 "ENTER = play again     LEFT / RIGHT = song     UP / DOWN = location",
                 d.score, d.best, d.newRecord ? "NEW RECORD!" : "", (int)std::round(d.accuracy * 100), d.grade, d.maxCombo,
                 d.perfect, d.good, d.miss);
        if (d.state == Game::State::Won) centeredBanner("won", "ENCORE! The crowd loved it", sub, kOn);
        else centeredBanner("lost", "The crowd went home...", sub, kOff);
    }

    ImDrawList* fg = ImGui::GetForegroundDrawList();
    ImFont* font = ImGui::GetFont();
    const float fs = ImGui::GetFontSize();

    if (playing && d.notes && d.noteStates) {
        const float laneW = 58.0f, height = 220.0f, lookAhead = 1.3f;
        const float pps = height / lookAhead;
        float x0 = io.DisplaySize.x * 0.5f - 2.0f * laneW;
        float yHit = io.DisplaySize.y - 64.0f, yTop = yHit - height;
        fg->AddRectFilled(ImVec2(x0 - 6, yTop), ImVec2(x0 + 4 * laneW + 6, yHit + 44), IM_COL32(0, 0, 0, 110), 8.0f);
        for (int i = 1; i < 4; ++i)
            fg->AddLine(ImVec2(x0 + i * laneW, yTop), ImVec2(x0 + i * laneW, yHit + 20), IM_COL32(255, 255, 255, 30));
        fg->AddLine(ImVec2(x0, yHit), ImVec2(x0 + 4 * laneW, yHit), IM_COL32(255, 255, 255, 170), 2.0f);
        for (int l = 0; l < 4; ++l) {
            ImVec2 c(x0 + (l + 0.5f) * laneW, yHit);
            float glow = std::max(d.laneDown[l] ? 0.6f : 0.0f, d.laneFlash[l]);
            if (glow > 0.0f) fg->AddCircleFilled(c, 26.0f, ImGui::GetColorU32(withAlpha(kLaneColor[l], 0.35f * glow)), 24);
            arrow(fg, c, 15.0f, l, ImGui::GetColorU32(withAlpha(kLaneColor[l], 0.6f + 0.4f * glow)), d.laneDown[l]);
            ImVec2 ts = ImGui::CalcTextSize(kLaneName[l]);
            fg->AddText(ImVec2(c.x - ts.x * 0.5f, yHit + 24), ImGui::GetColorU32(withAlpha(kLaneColor[l], 0.75f)), kLaneName[l]);
        }
        const auto& notes = *d.notes;
        const auto& st = *d.noteStates;
        for (int i = std::max(0, d.firstNote - 8); i < (int)notes.size(); ++i) {
            double dtNote = notes[i].time - d.songTime;
            if (dtNote > lookAhead) break;
            if (st[i] != Game::Pending) continue;
            float y = yHit - (float)dtNote * pps;
            if (y > yHit + 18.0f) continue;
            int l = notes[i].lane;
            ImVec2 c(x0 + (l + 0.5f) * laneW, y);
            float fade = std::min(1.0f, (y - yTop) / 40.0f);
            if (fade <= 0.0f) continue;
            fg->AddCircleFilled(c, 21.0f, ImGui::GetColorU32(withAlpha(kLaneColor[l], 0.25f * fade)), 20);
            arrow(fg, c, 16.0f, l, ImGui::GetColorU32(withAlpha(kLaneColor[l], fade)), true);
            arrow(fg, c, 16.0f, l, IM_COL32(255, 255, 255, (int)(160 * fade)), false);
        }
        if (d.judge != Game::Judge::None && d.judgeAge < 0.7f) {
            const bool perfect = d.judge == Game::Judge::Perfect, good = d.judge == Game::Judge::Good;
            const char* txt = perfect ? "PERFECT" : good ? "GOOD" : "MISS";
            ImVec4 col = perfect ? kGold : good ? kOn : kOff;
            float pop = 1.0f + 0.35f * std::max(0.0f, 1.0f - d.judgeAge * 8.0f);
            float a = std::min(1.0f, (0.7f - d.judgeAge) / 0.25f);
            float size = fs * 2.2f * pop;
            ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0.0f, txt);
            ImVec2 pos(io.DisplaySize.x * 0.5f - ts.x * 0.5f, yTop - ts.y - 34.0f);
            fg->AddText(font, size, ImVec2(pos.x + 2, pos.y + 2), IM_COL32(0, 0, 0, (int)(150 * a)), txt);
            fg->AddText(font, size, pos, ImGui::GetColorU32(withAlpha(col, a)), txt);
        }
        if (d.combo >= 4) {
            char txt[32];
            snprintf(txt, sizeof(txt), "%d COMBO", d.combo);
            float size = fs * 1.5f;
            ImVec2 ts = font->CalcTextSizeA(size, FLT_MAX, 0.0f, txt);
            ImVec2 pos(io.DisplaySize.x * 0.5f - ts.x * 0.5f, yTop - ts.y - 6.0f);
            fg->AddText(font, size, pos, IM_COL32(255, 255, 255, 220), txt);
        }
    }

    if (d.messageAlpha > 0.0f && !d.message.empty()) {
        if (playing) ImGui::SetNextWindowPos(ImVec2(12, io.DisplaySize.y - 12), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        else ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y - 24), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.6f * d.messageAlpha);
        ImGui::Begin("toast", nullptr, panel | ImGuiWindowFlags_NoInputs);
        ImGui::TextColored(ImVec4(1, 1, 1, d.messageAlpha), "%s", d.message.c_str());
        ImGui::End();
    }

    if (d.showHelp) {
        ImGui::SetNextWindowPos(ImVec2(12, io.DisplaySize.y - 12), ImGuiCond_Always, ImVec2(0, 1));
        ImGui::SetNextWindowBgAlpha(0.75f);
        ImGui::Begin("help", nullptr, panel);
        ImGui::TextColored(kGold, "Controls");
        ImGui::Separator();
        const char* rows[][2] = {
            {"Arrows  (or D F J K)", "hit notes: left lights / lasers / pyro / right lights"},
            {"Left / Right (menu)", "choose the song"},
            {"Up / Down (menu)", "choose the location"},
            {"Enter", "start / restart the show"},
            {"R", "reset"},
            {"Mouse drag / wheel", "rotate / zoom the camera"},
            {"Tab", "next camera view (5 views)"},
            {"C", "free-fly camera (WASD, Space/Ctrl, Shift = fast)"},
            {"O", "autopilot: perfect play (demo)"},
            {"M", "mute music"},
            {"I", "toggle crowd instancing (headline technique)"},
            {"1 2 3 4", "crowd size: 1 000 / 3 000 / 20 000 / 50 000"},
            {"T", "toggle wind animation"},
            {"P", "pause"},
            {"Q / E", "rotate the sun (azimuth)"},
            {"Z / X", "lower / raise the sun"},
            {"V", "toggle VSync (for FPS measurements)"},
            {"F5", "reload shaders from disk"},
            {"F12", "save screenshot (BMP)"},
            {"F1 / H", "show / hide this help"},
            {"Esc", "quit"},
        };
        if (ImGui::BeginTable("keys", 2)) {
            for (auto& r : rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(kGold, "%s", r[0]);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(r[1]);
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
}

void Hud::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
