#pragma once

#include <string>
#include <vector>

#include "4ZT/output/image.h"

namespace FourZT {

enum class Codec {
    Auto,   // picks the best available encoder
    H264,
    HEVC,
    AV1,
    VP9
};

struct VideoSettings {
    int fps = 24;
    int quality = 80; // 0-100, higher is better (mapped to CRF / bitrate per encoder)
    Codec codec = Codec::Auto;
};

class Video {
public:
    // Frame size is taken from the first frame, all frames must match it
    Video(std::vector<Image> frames, const VideoSettings& settings);

    bool Save(const std::string& filename) const;

private:
    std::vector<Image> m_frames;
    VideoSettings m_settings;

    // Shared libavcodec/libavformat pipeline (SDR only for now)
    bool Encode(const std::string& filename) const;
};

} // 4zt