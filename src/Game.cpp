#include "Game.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

Game::Game() { reloadChart(); }

void Game::reloadChart() {
    notes_ = music::chart();
    reset();
}

void Game::reset() {
    state_ = State::Ready;
    status_.assign(notes_.size(), Pending);
    next_ = score_ = combo_ = maxCombo_ = perfect_ = good_ = miss_ = 0;
    hype_ = 0.5f;
    newRecord_ = false;
    lastJudge_ = Judge::None;
    lastJudgeTime_ = -10.0;
    now_ = 0.0;
    events_.clear();
}

void Game::start() {
    if (state_ == State::Playing) return;
    if (state_ != State::Ready) reset();
    state_ = State::Playing;
}

void Game::skipTo(double t) {
    for (size_t i = 0; i < notes_.size(); ++i)
        if (notes_[i].time < t && status_[i] == Pending) status_[i] = Skipped;
    skipJudged();
}

void Game::skipJudged() {
    while (next_ < (int)notes_.size() && status_[next_] != Pending) ++next_;
}

void Game::loadBest(const std::string& path) {
    bestPath_ = path;
    std::ifstream f(path);
    if (f >> best_) std::cout << "[game] best score " << best_ << "\n";
    else best_ = 0;
}

float Game::accuracy() const {
    int judged = perfect_ + good_ + miss_;
    return judged ? (perfect_ + 0.5f * good_) / judged : 1.0f;
}

char Game::grade() const {
    float a = accuracy();
    return a >= 0.95f ? 'S' : a >= 0.85f ? 'A' : a >= 0.70f ? 'B' : a >= 0.50f ? 'C' : 'D';
}

std::vector<Game::Event> Game::takeEvents() {
    std::vector<Event> e;
    e.swap(events_);
    return e;
}

void Game::judge(int idx, double err) {
    int lane = notes_[idx].lane;
    double e = std::fabs(err);
    Judge j;
    if (e <= kPerfect) {
        j = Judge::Perfect;
        status_[idx] = HitPerfect;
        ++perfect_;
    } else if (e <= kGood) {
        j = Judge::Good;
        status_[idx] = HitGood;
        ++good_;
    } else {
        j = Judge::Miss;
        status_[idx] = Missed;
        ++miss_;
    }
    if (j == Judge::Miss) {
        combo_ = 0;
        hype_ -= 0.07f;
    } else {
        ++combo_;
        maxCombo_ = std::max(maxCombo_, combo_);
        float mult = 1.0f + std::min(combo_, 30) / 10.0f;
        score_ += (int)((j == Judge::Perfect ? 300 : 100) * mult);
        hype_ += j == Judge::Perfect ? 0.035f : 0.02f;
    }
    hype_ = std::clamp(hype_, 0.0f, 1.0f);
    lastJudge_ = j;
    lastJudgeTime_ = now_;
    events_.push_back({lane, j});
    skipJudged();
}

void Game::press(int lane, double t) {
    if (state_ != State::Playing) {
        events_.push_back({lane, Judge::None});
        return;
    }
    now_ = t;
    int bestIdx = -1;
    double bestErr = 1e9;
    for (int i = next_; i < (int)notes_.size() && notes_[i].time < t + kNear; ++i) {
        if (status_[i] != Pending || notes_[i].lane != lane) continue;
        double err = t - notes_[i].time;
        if (std::fabs(err) < std::fabs(bestErr)) {
            bestErr = err;
            bestIdx = i;
        }
    }
    if (bestIdx >= 0 && std::fabs(bestErr) <= kNear) {
        judge(bestIdx, bestErr);
    } else {
        hype_ = std::max(0.0f, hype_ - 0.015f);
        events_.push_back({lane, Judge::None});
    }
}

void Game::autoplay(double t) {
    if (state_ != State::Playing) return;
    while (next_ < (int)notes_.size() && notes_[next_].time <= t) press(notes_[next_].lane, notes_[next_].time);
}

void Game::update(double t) {
    if (state_ != State::Playing) return;
    now_ = t;
    for (int i = next_; i < (int)notes_.size() && notes_[i].time + kGood < t; ++i)
        if (status_[i] == Pending) judge(i, kNear + 1.0);
    if (hype_ <= 0.0f) finish(State::Lost);
    else if (t >= music::songEnd() + 2.0) finish(State::Won);
}

void Game::finish(State s) {
    state_ = s;
    std::cout << "[game] show over: " << (s == State::Won ? "WON" : "LOST") << ", score " << score_ << ", accuracy "
              << (int)std::round(accuracy() * 100) << "% (" << grade() << "), max combo " << maxCombo_ << "\n";
    if (score_ > best_) {
        best_ = score_;
        newRecord_ = true;
        if (bestPath_.empty()) return;
        std::ofstream f(bestPath_);
        if (f) f << best_ << "\n";
        else std::cerr << "[game] could not save best score to " << bestPath_ << "\n";
    }
}
