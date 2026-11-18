#pragma once
#include <atomic>
#include <memory>

class Audio {
public:
    Audio();
    ~Audio();
    bool init();
    void shutdown();
    bool ok() const { return ok_; }

    void play() { playing_ = true; }
    void pause() { playing_ = false; }
    bool playing() const { return playing_; }
    void seek(double seconds);
    double time() const;
    void setMuted(bool m) { muted_ = m; }
    bool muted() const { return muted_; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    bool ok_ = false;
    std::atomic<long long> frame_{0};
    std::atomic<bool> playing_{false}, muted_{false};
    friend struct AudioCallback;
};
