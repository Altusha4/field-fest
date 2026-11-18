#include "Screenshot.h"

#include <glad/gl.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

bool saveScreenshotBMP(const std::string& path, int w, int h) {
    std::vector<uint8_t> rgb((size_t)w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());

    int rowSize = (w * 3 + 3) & ~3;
    uint32_t dataSize = (uint32_t)rowSize * h, fileSize = 54 + dataSize;
    uint8_t hdr[54] = {'B', 'M'};
    auto put32 = [&](int off, uint32_t v) { for (int i = 0; i < 4; ++i) hdr[off + i] = (uint8_t)(v >> (8 * i)); };
    put32(2, fileSize);
    put32(10, 54);
    put32(14, 40);
    put32(18, (uint32_t)w);
    put32(22, (uint32_t)h);
    hdr[26] = 1;
    hdr[28] = 24;
    put32(34, dataSize);

    std::ofstream f(path, std::ios::binary);
    if (!f) {
        std::cerr << "[screenshot] cannot write " << path << "\n";
        return false;
    }
    f.write((const char*)hdr, 54);
    std::vector<uint8_t> row(rowSize, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* p = &rgb[((size_t)y * w + x) * 3];
            row[x * 3 + 0] = p[2];
            row[x * 3 + 1] = p[1];
            row[x * 3 + 2] = p[0];
        }
        f.write((const char*)row.data(), rowSize);
    }
    std::cout << "[screenshot] saved " << path << " (" << w << "x" << h << ")\n";
    return true;
}
