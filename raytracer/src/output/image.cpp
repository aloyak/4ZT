// 4ZT/output/image.cpp
#include "4ZT/output/image.h"
#include "4ZT/output/color_space.h"

#include <fstream>
#include <iostream>

static const Interval intensity(0.000, 0.999);

// Linear scene value -> 8-bit display value.
// Encode to sRGB first, then clamp (so 0.999 maps to 255 after *256), then quantize.
static int ToByte(double linear) {
    return int(256 * intensity.Clamp(FourZT::ColorSpace::LinearToSRGB(linear)));
}

void FourZT::Image::WritePPM(std::ostream& out) const {
    out << "P3\n" << m_buffer.width() << ' ' << m_buffer.height() << "\n255\n";

    for (int y = m_buffer.height() - 1; y >= 0; --y) {
        for (int x = 0; x < m_buffer.width(); ++x) {
            const Color& c = at(x, y);
            out << ToByte(c.x) << ' ' << ToByte(c.y) << ' ' << ToByte(c.z) << '\n';
        }
    }
}

bool FourZT::Image::Save(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Could not open file for writing: " << filename << std::endl;
        return false;
    }
    WritePPM(outFile);
    return bool(outFile);
}

void FourZT::Image::PrintOut() const {
    WritePPM(std::cout);
}