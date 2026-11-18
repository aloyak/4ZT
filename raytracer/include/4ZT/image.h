#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

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
    void Save(const std::string& filename) const {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Could not open file for writing: " << filename << std::endl;
            return;
        }

        // Write the PPM header
        outFile << "P3\n" << m_buffer.width() << ' ' << m_buffer.height() << "\n255\n";

        // Write pixel data
        for (int y = m_buffer.height() - 1; y >= 0; --y) {
            for (int x = 0; x < m_buffer.width(); ++x) {
                const Color& pixelColor = at(x, y);

                // Translate [0,1] to [0,255].
                int rbyte = int(255.999 * pixelColor.x);
                int gbyte = int(255.999 * pixelColor.y);
                int bbyte = int(255.999 * pixelColor.z);

                outFile << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
            }
        }

        outFile.close();
        std::cout << "Image saved to " << filename << std::endl;
    }

    // Prints out the image in PPM format to stdout, easy to redirect to a file, used for testing
    void PrintOut() const {
        std::cout << "P3\n" << m_buffer.width() << ' ' << m_buffer.height() << "\n255\n";

        for (int y = m_buffer.height() - 1; y >= 0; --y) {
            for (int x = 0; x < m_buffer.width(); ++x) {
                const Color& pixelColor = at(x, y);

                // Translate [0,1] to [0,255].
                int rbyte = int(255.999 * pixelColor.x);
                int gbyte = int(255.999 * pixelColor.y);
                int bbyte = int(255.999 * pixelColor.z);

                std::cout << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
            }
        }   
    }

private:
    Buffer<Color> m_buffer;
};

} // 4zt