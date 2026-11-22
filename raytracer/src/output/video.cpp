#include "4ZT/output/video.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace FourZT {

namespace {

struct FormatDeleter {
    void operator()(AVFormatContext* c) const {
        if (!c) return;
        if (c->pb && !(c->oformat->flags & AVFMT_NOFILE)) avio_closep(&c->pb);
        avformat_free_context(c);
    }
};
struct CodecDeleter  { void operator()(AVCodecContext* c) const { avcodec_free_context(&c); } };
struct FrameDeleter  { void operator()(AVFrame* f)        const { av_frame_free(&f); } };
struct PacketDeleter { void operator()(AVPacket* p)       const { av_packet_free(&p); } };
struct SwsDeleter    { void operator()(SwsContext* s)     const { sws_freeContext(s); } };

using FormatPtr = std::unique_ptr<AVFormatContext, FormatDeleter>;
using CodecPtr  = std::unique_ptr<AVCodecContext, CodecDeleter>;
using FramePtr  = std::unique_ptr<AVFrame, FrameDeleter>;
using PacketPtr = std::unique_ptr<AVPacket, PacketDeleter>;
using SwsPtr    = std::unique_ptr<SwsContext, SwsDeleter>;

std::string AvError(int err) {
    char buf[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(err, buf, sizeof(buf));
    return buf;
}

std::string ExtensionOf(const std::string& filename) {
    const size_t dot = filename.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string ext = filename.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

// First encoder from the list that is compiled into the linked FFmpeg
const AVCodec* FirstAvailable(std::initializer_list<const char*> names) {
    for (const char* name : names) {
        if (const AVCodec* c = avcodec_find_encoder_by_name(name)) return c;
    }
    return nullptr;
}

const AVCodec* FindEncoder(Codec codec) {
    auto h264 = [] { return FirstAvailable({"libx264", "h264_nvenc", "h264_videotoolbox", "h264_qsv", "h264_amf"}); };
    auto hevc = [] { return FirstAvailable({"libx265", "hevc_nvenc", "hevc_videotoolbox", "hevc_qsv", "hevc_amf"}); };
    auto av1  = [] { return FirstAvailable({"libsvtav1", "libaom-av1", "librav1e", "av1_nvenc", "av1_qsv", "av1_amf"}); };
    auto vp9  = [] { return FirstAvailable({"libvpx-vp9"}); };

    switch (codec) {
        case Codec::H264: return h264();
        case Codec::HEVC: return hevc();
        case Codec::AV1:  return av1();
        case Codec::VP9:  return vp9();
        case Codec::Auto: break;
    }

    // Auto: widest compatibility first
    const AVCodec* c = nullptr;
    if ((c = h264())) return c;
    if ((c = av1())) return c;
    return vp9();
}

bool NameContains(const AVCodec* codec, const char* part) {
    return std::string(codec->name).find(part) != std::string::npos;
}

bool IsHardwareEncoder(const AVCodec* codec) {
    return NameContains(codec, "nvenc") || NameContains(codec, "videotoolbox") ||
           NameContains(codec, "qsv")   || NameContains(codec, "amf");
}

// quality 0-100 -> encoder specific rate control. Must be called before avcodec_open2.
void ApplyQuality(AVCodecContext* ctx, const AVCodec* codec, int quality, int fps) {
    quality = std::clamp(quality, 0, 100);
    const double q = quality / 100.0;
    void* priv = ctx->priv_data;

    if (NameContains(codec, "libx264") || NameContains(codec, "libx265")) {
        av_opt_set_int(priv, "crf", std::lround(51.0 * (1.0 - q)), 0);
    } else if (NameContains(codec, "libsvtav1")) {
        av_opt_set_int(priv, "crf", std::lround(63.0 * (1.0 - q)), 0);
    } else if (NameContains(codec, "libaom") || NameContains(codec, "libvpx")) {
        ctx->bit_rate = 0; // constant-quality mode needs bitrate 0
        av_opt_set_int(priv, "crf", std::lround(63.0 * (1.0 - q)), 0);
        av_opt_set_int(priv, "cpu-used", NameContains(codec, "libaom") ? 6 : 4, 0);
        av_opt_set_int(priv, "row-mt", 1, 0);
    } else {
        // Hardware encoders and anything else: fall back to a bits-per-pixel target
        const double bpp = 0.02 + 0.23 * q;
        ctx->bit_rate = static_cast<int64_t>(double(ctx->width) * ctx->height * fps * bpp);
    }
}

} // namespace

Video::Video(std::vector<Image> frames, const VideoSettings& settings)
    : m_frames(std::move(frames)), m_settings(settings) {}

bool Video::Save(const std::string& filename) const {
    const std::string ext = ExtensionOf(filename);

    if (ext == ".mp4" || ext == ".mkv" || ext == ".gif") {
        return Encode(filename);
    }

    std::cerr << "Video: unsupported file extension '" << ext << "' (use .mp4, .mkv or .gif)\n";
    return false;
}

bool Video::Encode(const std::string& filename) const {
    if (m_frames.empty()) {
        std::cerr << "Video: no frames to encode\n";
        return false;
    }

    const int srcW = m_frames[0].width();
    const int srcH = m_frames[0].height();
    for (const Image& f : m_frames) {
        if (f.width() != srcW || f.height() != srcH) {
            std::cerr << "Video: all frames must have the same size\n";
            return false;
        }
    }

    // 4:2:0 needs even dimensions, crop at most one pixel row/column
    const int w = srcW & ~1;
    const int h = srcH & ~1;
    if (w < 2 || h < 2) {
        std::cerr << "Video: frame size too small\n";
        return false;
    }

    const int fps = std::max(1, m_settings.fps);

    // container
    AVFormatContext* rawFmt = nullptr;
    int ret = avformat_alloc_output_context2(&rawFmt, nullptr, nullptr, filename.c_str());
    if (ret < 0 || !rawFmt) {
        std::cerr << "Video: could not create output context: " << AvError(ret) << "\n";
        return false;
    }
    FormatPtr fmt(rawFmt);

    const bool gif = fmt->oformat->video_codec == AV_CODEC_ID_GIF;

    // encoder
    const AVCodec* codec = gif ? avcodec_find_encoder(AV_CODEC_ID_GIF)
                               : FindEncoder(m_settings.codec);
    if (!codec) {
        std::cerr << "Video: no suitable encoder is built into the linked FFmpeg\n";
        return false;
    }

    // Pixel format going into the encoder.
    // GIF uses rgb8 (fixed 3-3-2 palette), swscale can't output pal8 directly.
    AVPixelFormat encFmt;
    if (gif)                           encFmt = AV_PIX_FMT_RGB8;
    else if (IsHardwareEncoder(codec)) encFmt = AV_PIX_FMT_NV12;
    else                               encFmt = AV_PIX_FMT_YUV420P;

    CodecPtr ctx(avcodec_alloc_context3(codec));
    if (!ctx) {
        std::cerr << "Video: could not allocate codec context\n";
        return false;
    }
    ctx->width = w;
    ctx->height = h;
    ctx->pix_fmt = encFmt;
    ctx->time_base = AVRational{1, fps};
    ctx->framerate = AVRational{fps, 1};
    ctx->sample_aspect_ratio = AVRational{1, 1};

    if (!gif) {
        ctx->gop_size = fps * 2;
        ctx->color_primaries = AVCOL_PRI_BT709;
        ctx->color_trc       = AVCOL_TRC_BT709;
        ctx->colorspace      = AVCOL_SPC_BT709;
        ctx->color_range     = AVCOL_RANGE_MPEG;
        ApplyQuality(ctx.get(), codec, m_settings.quality, fps);
    }

    if (fmt->oformat->flags & AVFMT_GLOBALHEADER) {
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_open2(ctx.get(), codec, nullptr);
    if (ret < 0) {
        std::cerr << "Video: could not open encoder '" << codec->name << "': " << AvError(ret) << "\n";
        return false;
    }

    AVStream* stream = avformat_new_stream(fmt.get(), nullptr);
    if (!stream) {
        std::cerr << "Video: could not create stream\n";
        return false;
    }
    stream->time_base = ctx->time_base;
    stream->avg_frame_rate = ctx->framerate;
    avcodec_parameters_from_context(stream->codecpar, ctx.get());

    // Apple players refuse HEVC in mp4 unless it is tagged hvc1
    if (codec->id == AV_CODEC_ID_HEVC) {
        stream->codecpar->codec_tag = MKTAG('h', 'v', 'c', '1');
    }

    // output file
    if (!(fmt->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&fmt->pb, filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            std::cerr << "Video: could not open '" << filename << "': " << AvError(ret) << "\n";
            return false;
        }
    }
    ret = avformat_write_header(fmt.get(), nullptr);
    if (ret < 0) {
        std::cerr << "Video: could not write header: " << AvError(ret) << "\n";
        return false;
    }

    // RGB24 -> encoder pixel format conversion
    SwsPtr sws(sws_getContext(w, h, AV_PIX_FMT_RGB24, w, h, encFmt,
                              SWS_BICUBIC, nullptr, nullptr, nullptr));
    if (!sws) {
        std::cerr << "Video: could not create pixel format converter\n";
        return false;
    }
    if (!gif) {
        // Full-range RGB in, limited-range YCbCr out, using the BT.709 matrix matching the tags above
        const int* coeffs = sws_getCoefficients(SWS_CS_ITU709);
        sws_setColorspaceDetails(sws.get(), coeffs, 1, coeffs, 0, 0, 1 << 16, 1 << 16);
    }

    FramePtr frame(av_frame_alloc());
    PacketPtr packet(av_packet_alloc());
    if (!frame || !packet) {
        std::cerr << "Video: out of memory\n";
        return false;
    }
    frame->format = encFmt;
    frame->width = w;
    frame->height = h;
    ret = av_frame_get_buffer(frame.get(), 0);
    if (ret < 0) {
        std::cerr << "Video: could not allocate frame buffer: " << AvError(ret) << "\n";
        return false;
    }

    // Sends one frame (or nullptr to flush) and writes out every packet the encoder returns
    auto encode = [&](AVFrame* f) -> bool {
        int r = avcodec_send_frame(ctx.get(), f);
        if (r < 0) {
            std::cerr << "Video: encoder rejected frame: " << AvError(r) << "\n";
            return false;
        }
        while (true) {
            r = avcodec_receive_packet(ctx.get(), packet.get());
            if (r == AVERROR(EAGAIN) || r == AVERROR_EOF) break;
            if (r < 0) {
                std::cerr << "Video: encoding failed: " << AvError(r) << "\n";
                return false;
            }
            av_packet_rescale_ts(packet.get(), ctx->time_base, stream->time_base);
            packet->stream_index = stream->index;
            r = av_interleaved_write_frame(fmt.get(), packet.get());
            if (r < 0) {
                std::cerr << "Video: could not write packet: " << AvError(r) << "\n";
                return false;
            }
        }
        return true;
    };

    // main loop
    const int srcStride = srcW * 3;
    for (size_t i = 0; i < m_frames.size(); ++i) {
        if (av_frame_make_writable(frame.get()) < 0) return false;

        // Top-down, sRGB-encoded 8-bit RGB, one frame at a time
        const std::vector<unsigned char> rgb = m_frames[i].ToRGB8();

        const uint8_t* srcData[4]  = { rgb.data(), nullptr, nullptr, nullptr };
        const int      srcLines[4] = { srcStride, 0, 0, 0 };
        sws_scale(sws.get(), srcData, srcLines, 0, h, frame->data, frame->linesize);

        frame->pts = static_cast<int64_t>(i);
        if (!encode(frame.get())) return false;
    }

    if (!encode(nullptr)) return false; // flush

    ret = av_write_trailer(fmt.get());
    if (ret < 0) {
        std::cerr << "Video: could not finalize file: " << AvError(ret) << "\n";
        return false;
    }
    return true;
}

} // 4zt