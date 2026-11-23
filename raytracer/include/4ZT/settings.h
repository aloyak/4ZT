#pragma once

namespace FourZT {

enum class Backend { CPU, GPU };

struct Settings {
    // General
    Backend backend = Backend::CPU;
    
    int samplesPerPixel = 100; // SSAA
    int maxDepth = 50;
    int minDepth = 5;

    // Image Settings
    int outputHeight = 600;
    int outputWidth = 800; // will be calculated from aspect ratio if not set
    double aspectRatio = 16.0 / 9.0;
    
    // CPU Rendering
    int targetThreads = 4;

    // GPU Rendering

    // Advanced
    bool enableBVH = true;
    bool enableDenoising = true; // TODO
    bool enableRussianRoulette = true;
};

}