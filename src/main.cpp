#include "App.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

int main(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        auto next = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "missing value for " << name << "\n";
                std::exit(2);
            }
            return argv[++i];
        };
        if (!std::strcmp(argv[i], "--shot")) o.shotPath = next("--shot");
        else if (!std::strcmp(argv[i], "--frames")) o.shotFrame = std::atoi(next("--frames"));
        else if (!std::strcmp(argv[i], "--elapsed")) o.elapsed = (float)std::atof(next("--elapsed"));
        else if (!std::strcmp(argv[i], "--view")) o.view = std::atoi(next("--view"));
        else if (!std::strcmp(argv[i], "--mute")) o.mute = true;
        else if (!std::strcmp(argv[i], "--nohud")) o.noHud = true;
        else if (!std::strcmp(argv[i], "--crowd")) o.crowd = std::atoi(next("--crowd"));
        else if (!std::strcmp(argv[i], "--bench")) o.bench = true;
        else if (!std::strcmp(argv[i], "--location")) o.location = std::atoi(next("--location"));
        else if (!std::strcmp(argv[i], "--song")) o.song = std::atoi(next("--song"));
        else if (!std::strcmp(argv[i], "--naive")) o.naive = true;
        else if (!std::strcmp(argv[i], "--autopilot")) o.autopilot = true;
        else if (!std::strcmp(argv[i], "--size")) {
            o.width = std::atoi(next("--size"));
            o.height = std::atoi(next("--size"));
        } else {
            std::cout << "usage: FieldFest [--shot out.bmp] [--frames N] [--elapsed seconds]\n"
                         "                 [--song N] [--location 0-2] [--view 0-4] [--crowd N] [--bench] [--naive] [--autopilot] [--mute] [--nohud] [--size W H]\n";
            return 0;
        }
    }
    App app(o);
    return app.run();
}
