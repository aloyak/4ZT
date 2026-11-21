#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "4ZT/math/math.h"

namespace FourZT {

template <typename Pixel>
class Buffer {
public:
    // Initialize the pixel buffer (vector) with the final amount of pixels on the image
    Buffer(int w, int h) : m_width(w), m_height(h), m_data(size_t(w) * h) {}

    Pixel&       at(int x, int y)       { return m_data[size_t(y) * m_width + x]; }
    const Pixel& at(int x, int y) const { return m_data[size_t(y) * m_width + x]; }

    int width()  const { return m_width; }
    int height() const { return m_height; }

    Pixel*       data()       { return m_data.data(); }
    const Pixel* data() const { return m_data.data(); }
    size_t size_bytes() const { return m_data.size() * sizeof(Pixel); }

private:
    int m_width, m_height;
    std::vector<Pixel> m_data;
};

class Image {
public:
    Image(int w, int h) : m_buffer(w, h) {}
    virtual ~Image() = default;

    // color C set at pixel (x,y) can be done like image.at(x,y) = C
    Color&       at(int x, int y)       { return m_buffer.at(x, y); }
    const Color& at(int x, int y) const { return m_buffer.at(x, y); }

    int width()  const { return m_buffer.width(); }
    int height() const { return m_buffer.height(); }

    // Assumes the file type from the extension, defaults to ppm because its easy
    /// TODO: jpg, png and bmp
    void Save(const std::string& filename) const;

    // Prints out the image in PPM format to stdout, easy to redirect to a file, used for testing
    void PrintOut() const;

private:
    Buffer<Color> m_buffer;
};

} // 4zt