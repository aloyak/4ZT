#include "4ZT/image.h"

void FourZT::Image::Save(const std::string& filename) const {
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
}

void FourZT::Image::PrintOut() const {
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