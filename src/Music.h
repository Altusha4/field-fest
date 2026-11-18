#pragma once
#include <string>
#include <vector>

namespace music {

constexpr int kBars = 48;
constexpr int kSampleRate = 48000;

struct Song {
    const char* title;
    const char* style;
    const char* difficulty;
    double bpm;
    int bassRoot[4];
    int chord[4][3];
    int level;
    float kickTop;
    float kickGain;
    float clapDecay;
    float clapTone;
    float swing;
    int bassMode;
    int bassHarmonics;
    bool softKeys;
    bool busyHats;
    float padGain;
    float hueShift;
    int track = -1;
};

int loadTracks(const std::string& folder);

int songCount();
const Song& song(int i);
void setSong(int i);
int currentSong();

double beat();
double bar();
double songEnd();
double length();
double durationOf(int i);

enum class Section { Intro, Build, Drop, Break, Build2, Drop2, Outro };
Section sectionAt(double t);
const char* sectionName(Section s);

struct Levels {
    float kick = 0, snare = 0, hat = 0, bass = 0, lead = 0, pad = 0;
    float beats = 0;
    float kickAge = 9;
    int bar = 0;
    Section section = Section::Intro;
};
Levels levels(double t);

void sample(double t, long long n, float& left, float& right);

struct Note {
    double time;
    int lane;
};
std::vector<Note> chart();

}
