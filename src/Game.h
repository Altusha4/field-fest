#pragma once
#include "Music.h"

#include <cstdint>
#include <string>
#include <vector>

class Game {
public:
    enum class State { Ready, Playing, Won, Lost };
    enum class Judge { None, Perfect, Good, Miss };
    enum NoteState : uint8_t { Pending, HitPerfect, HitGood, Missed, Skipped };
    static constexpr double kPerfect = 0.055;
    static constexpr double kGood = 0.120;
    static constexpr double kNear = 0.200;

    struct Event {
        int lane;
        Judge judge;
    };

    Game();
    void reloadChart();
    void reset();
    void start();
    void skipTo(double songTime);
    void update(double songTime);
    void press(int lane, double songTime);
    void autoplay(double songTime);
    std::vector<Event> takeEvents();
    void loadBest(const std::string& path);

    State state() const { return state_; }
    int score() const { return score_; }
    int best() const { return best_; }
    bool newRecord() const { return newRecord_; }
    int combo() const { return combo_; }
    int maxCombo() const { return maxCombo_; }
    int perfect() const { return perfect_; }
    int good() const { return good_; }
    int miss() const { return miss_; }
    float hype() const { return hype_; }
    float accuracy() const;
    char grade() const;
    Judge lastJudge() const { return lastJudge_; }
    double lastJudgeTime() const { return lastJudgeTime_; }
    const std::vector<music::Note>& notes() const { return notes_; }
    const std::vector<uint8_t>& noteStates() const { return status_; }
    int firstPending() const { return next_; }

private:
    void judge(int idx, double err);
    void skipJudged();
    void finish(State s);

    State state_ = State::Ready;
    std::vector<music::Note> notes_;
    std::vector<uint8_t> status_;
    int next_ = 0;
    int score_ = 0, best_ = 0, combo_ = 0, maxCombo_ = 0, perfect_ = 0, good_ = 0, miss_ = 0;
    float hype_ = 0.5f;
    bool newRecord_ = false;
    Judge lastJudge_ = Judge::None;
    double lastJudgeTime_ = -10.0, now_ = 0.0;
    std::vector<Event> events_;
    std::string bestPath_;
};
