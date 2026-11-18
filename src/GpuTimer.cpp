#include "GpuTimer.h"

void GpuTimer::init() { glGenQueries(kRing, q_); }

void GpuTimer::begin() { glBeginQuery(GL_TIME_ELAPSED, q_[idx_]); }

void GpuTimer::end() {
    glEndQuery(GL_TIME_ELAPSED);
    ++issued_;
    idx_ = (idx_ + 1) % kRing;
    if (issued_ >= kRing) {
        GLint available = 0;
        glGetQueryObjectiv(q_[idx_], GL_QUERY_RESULT_AVAILABLE, &available);
        if (available) {
            GLuint64 ns = 0;
            glGetQueryObjectui64v(q_[idx_], GL_QUERY_RESULT, &ns);
            ms_ = ms_ * 0.9 + (ns / 1.0e6) * 0.1;
        }
    }
}

void GpuTimer::release() { glDeleteQueries(kRing, q_); }
