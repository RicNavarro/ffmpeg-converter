#include "FFmpegPresets.h"

// ---------------------------------------------------------------------------
// Available formats (shown in UI combo box)
// ---------------------------------------------------------------------------

QList<FormatOption> FFmpegPresets::availableFormats()
{
    return {
        // Video
        { "mp4",  "MP4 — H.264 / AAC",        "Video" },
        { "mp4",  "MP4 — H.265 / AAC",        "Video" },   // handled via codec field
        { "mkv",  "MKV — H.264 / AAC",        "Video" },
        { "mkv",  "MKV — H.265 / AAC",        "Video" },
        { "webm", "WebM — VP9 / Opus",         "Video" },
        { "avi",  "AVI — MPEG-4 / MP3",        "Video" },
        { "mov",  "MOV — H.264 / AAC",         "Video" },
        { "gif",  "GIF — Animated",            "Video" },

        // Audio
        { "mp3",  "MP3 — MPEG Layer 3",        "Audio" },
        { "aac",  "AAC — Advanced Audio",      "Audio" },
        { "flac", "FLAC — Lossless",           "Audio" },
        { "ogg",  "OGG — Vorbis",              "Audio" },
        { "wav",  "WAV — PCM Uncompressed",    "Audio" },
        { "m4a",  "M4A — AAC in MPEG-4",       "Audio" },
        { "opus", "Opus — Low-latency audio",  "Audio" },
    };
}

// ---------------------------------------------------------------------------
// Codec lists per format  (for SettingsPanel dropdowns)
// ---------------------------------------------------------------------------

QStringList FFmpegPresets::videoCodecsFor(const QString &format)
{
    const QString f = format.toLower();

    if (f == "mp4" || f == "mov")
        return { "libx264", "libx265", "copy" };

    if (f == "mkv")
        return { "libx264", "libx265", "libvpx-vp9", "copy" };

    if (f == "webm")
        return { "libvpx-vp9", "libvpx" };

    if (f == "avi")
        return { "mpeg4", "libx264", "copy" };

    if (f == "gif")
        return {};    // GIF has no separate video codec concept

    return { "copy" };
}

QStringList FFmpegPresets::audioCodecsFor(const QString &format)
{
    const QString f = format.toLower();

    if (f == "mp3")  return { "libmp3lame" };
    if (f == "aac")  return { "aac", "libfdk_aac" };
    if (f == "flac") return { "flac" };
    if (f == "ogg")  return { "libvorbis" };
    if (f == "wav")  return { "pcm_s16le", "pcm_s24le", "pcm_f32le" };
    if (f == "m4a")  return { "aac", "libfdk_aac" };
    if (f == "opus") return { "libopus" };

    // Video containers
    if (f == "mp4" || f == "mov") return { "aac", "libmp3lame", "copy" };
    if (f == "mkv")               return { "aac", "libmp3lame", "libvorbis", "libopus", "copy" };
    if (f == "webm")              return { "libopus", "libvorbis" };
    if (f == "avi")               return { "libmp3lame", "copy" };
    if (f == "gif")               return {};

    return { "copy" };
}

// ---------------------------------------------------------------------------
// Baseline args — sane defaults before user overrides
// ---------------------------------------------------------------------------

QStringList FFmpegPresets::baseArgs(const QString &format,
                                    const QString &videoCodec,
                                    const QString &audioCodec)
{
    const QString f = format.toLower();

    // --- GIF (special pipeline) ---
    if (f == "gif") {
        return {
            "-vf", "fps=15,scale=480:-1:flags=lanczos,split[s0][s1];"
                   "[s0]palettegen[p];[s1][p]paletteuse",
            "-loop", "0"
        };
    }

    // --- Audio-only formats ---
    if (f == "mp3")  return { "-c:a", "libmp3lame", "-q:a", "2", "-vn" };
    if (f == "flac") return { "-c:a", "flac", "-vn" };
    if (f == "wav")  return { "-c:a", "pcm_s16le", "-vn" };
    if (f == "ogg")  return { "-c:a", "libvorbis", "-q:a", "4", "-vn" };
    if (f == "opus") return { "-c:a", "libopus", "-b:a", "128k", "-vn" };
    if (f == "aac")  return { "-c:a", "aac", "-b:a", "192k", "-vn" };
    if (f == "m4a")  return { "-c:a", "aac", "-b:a", "192k", "-vn" };

    // --- Video containers ---
    QStringList args;

    // Video codec
    const QString vc = videoCodec.isEmpty() ? "libx264" : videoCodec;
    args << "-c:v" << vc;

    if (vc == "libx264" || vc == "libx265") {
        args << "-crf" << (vc == "libx265" ? "28" : "23");
        args << "-preset" << "medium";
    } else if (vc == "libvpx-vp9") {
        args << "-crf" << "33" << "-b:v" << "0";
    } else if (vc == "mpeg4") {
        args << "-q:v" << "6";
    }

    // Audio codec
    const QString ac = audioCodec.isEmpty() ? "aac" : audioCodec;
    if (f == "webm") {
        args << "-c:a" << (ac == "copy" ? "libopus" : ac);
    } else {
        args << "-c:a" << ac;
        if (ac == "aac")          args << "-b:a" << "192k";
        else if (ac == "libmp3lame") args << "-b:a" << "192k";
    }

    // Container-specific flags
    if (f == "mp4" || f == "mov")
        args << "-movflags" << "+faststart";

    return args;
}

// ---------------------------------------------------------------------------
// buildArgs — applies user overrides on top of base args
// ---------------------------------------------------------------------------

QStringList FFmpegPresets::buildArgs(const FormatProfile &profile)
{
    const QString vc = profile.videoCodec;
    const QString ac = profile.audioCodec;

    QStringList args = baseArgs(profile.outputFormat, vc, ac);

    // Override CRF if user set it explicitly
    if (profile.crf >= 0) {
        int crfIdx = args.indexOf("-crf");
        if (crfIdx != -1 && crfIdx + 1 < args.size())
            args[crfIdx + 1] = QString::number(profile.crf);
        else
            args << "-crf" << QString::number(profile.crf);
    }

    // Override video bitrate (removes CRF if present, uses -b:v instead)
    if (profile.videoBitrate > 0) {
        int crfIdx = args.indexOf("-crf");
        if (crfIdx != -1) {
            args.removeAt(crfIdx + 1);
            args.removeAt(crfIdx);
        }
        int bvIdx = args.indexOf("-b:v");
        if (bvIdx != -1 && bvIdx + 1 < args.size())
            args[bvIdx + 1] = QString::number(profile.videoBitrate) + "k";
        else
            args << "-b:v" << QString::number(profile.videoBitrate) + "k";
    }

    // Override audio bitrate
    if (profile.audioBitrate > 0) {
        int baIdx = args.indexOf("-b:a");
        if (baIdx != -1 && baIdx + 1 < args.size())
            args[baIdx + 1] = QString::number(profile.audioBitrate) + "k";
        else
            args << "-b:a" << QString::number(profile.audioBitrate) + "k";
    }

    // Resolution scaling
    if (!profile.resolution.isEmpty() && profile.outputFormat != "gif") {
        // resolution is stored as "1920x1080" — convert to ffmpeg scale filter
        QString res = profile.resolution;
        res.replace("x", ":");
        // Append to existing -vf or create new one
        int vfIdx = args.indexOf("-vf");
        if (vfIdx != -1 && vfIdx + 1 < args.size())
            args[vfIdx + 1] = "scale=" + res + "," + args[vfIdx + 1];
        else
            args << "-vf" << "scale=" + res;
    }

    return args;
}
