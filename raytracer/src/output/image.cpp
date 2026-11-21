// 4ZT/output/image.cpp
#include "4ZT/output/image.h"
#include "4ZT/output/color_space.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static const Interval intensity(0.000, 0.999);

// Linear scene value -> 8-bit display value.
// Encode to sRGB first, then clamp (so 0.999 maps to 255 after *256), then quantize.
static int ToByte(double linear) {
    return int(256 * intensity.Clamp(FourZT::ColorSpace::LinearToSRGB(linear)));
}

static std::string LowerExtension(const std::string& filename) {
    std::string ext = std::filesystem::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    return ext;
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

void FourZT::Image::PrintOut() const {
    WritePPM(std::cout);
}

// buffer builders
// makes a top-down, sRGB-encoded 8-bit RGB buffer for PNG/JPG
std::vector<unsigned char> FourZT::Image::ToRGB8() const {
    std::vector<unsigned char> out;
    out.reserve(size_t(width()) * height() * 3);
    for (int y = height() - 1; y >= 0; --y) {          // top row first
        for (int x = 0; x < width(); ++x) {
            const Color& c = at(x, y);
            out.push_back((unsigned char)ToByte(c.x));
            out.push_back((unsigned char)ToByte(c.y));
            out.push_back((unsigned char)ToByte(c.z));
        }
    }
    return out;
}

// makes a linear float RGB buffer, top-down, for HDR/EXR
std::vector<float> FourZT::Image::ToRGBFloat() const {
    std::vector<float> out;
    out.reserve(size_t(width()) * height() * 3);
    for (int y = height() - 1; y >= 0; --y) {
        for (int x = 0; x < width(); ++x) {
            const Color& c = at(x, y);
            out.push_back(float(c.x));                  // linear, unclamped
            out.push_back(float(c.y));
            out.push_back(float(c.z));
        }
    }
    return out;
}
// ----

bool FourZT::Image::Save(const std::string& filename, int jpgQuality) const {
    const std::string ext = LowerExtension(filename);
    bool success = false;

    if      (ext == ".ppm") success = SavePPM(filename);
    else if (ext == ".png") success = SavePNG(filename);
    else if (ext == ".hdr") success = SaveHDR(filename);
    else if (ext == ".exr") success = SaveEXR(filename);
    else if (ext == ".jpg" || ext == ".jpeg") success = SaveJPG(filename, jpgQuality);
    else {
        std::cerr << "Error: Unknown image file extension '" << ext << "'\n";
    }

    if (!success) std::cerr << "Failed to write image: " << filename << std::endl;
    return success;
}

bool FourZT::Image::SavePPM(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) return false;
    WritePPM(outFile);
    return bool(outFile);
}

bool FourZT::Image::SavePNG(const std::string& filename) const {
    auto pixels = ToRGB8();
    return stbi_write_png(filename.c_str(), width(), height(), 3,
                          pixels.data(), width() * 3) != 0;
}

bool FourZT::Image::SaveJPG(const std::string& filename, int quality) const {
    auto pixels = ToRGB8();
    return stbi_write_jpg(filename.c_str(), width(), height(), 3,
                          pixels.data(), quality) != 0;
}

bool FourZT::Image::SaveHDR(const std::string& filename) const {
    auto pixels = ToRGBFloat();
    return stbi_write_hdr(filename.c_str(), width(), height(), 3, pixels.data()) != 0;
}

bool FourZT::Image::SaveEXR(const std::string& filename) const {
    std::cerr << "Error: EXR saving not implemented yet!\n";
    return false;
}