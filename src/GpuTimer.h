#pragma once
#include <glad/gl.h>

class GpuTimer {
public:
    void init();
    void begin();
    void end();
    double milliseconds() const { return ms_; }
    void release();

private:
    static constexpr int kRing = 4;
    GLuint q_[kRing] = {};
    int idx_ = 0, issued_ = 0;
    double ms_ = 0.0;
};
