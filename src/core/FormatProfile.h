#pragma once

#include <QString>
#include <QStringList>

// Represents a single conversion job
struct FormatProfile {
    // --- Input/Output ---
    QString inputPath;
    QString outputDir;
    QString outputFormat;   // e.g. "mp4", "mp3", "mkv"

    // --- Video settings (ignored for audio-only formats) ---
    QString videoCodec;     // e.g. "libx264", "libvpx-vp9", "" = copy
    int     videoBitrate = 0;       // kbps, 0 = use CRF / default
    int     crf         = -1;       // -1 = not set
    QString resolution;     // e.g. "1920x1080", "" = original

    // --- Audio settings ---
    QString audioCodec;     // e.g. "aac", "libmp3lame", "" = copy
    int     audioBitrate = 0;       // kbps, 0 = default

    // --- Derived ---
    // Builds the full output file path: outputDir/baseName.outputFormat
    QString outputPath() const;

    // Returns true if the format is audio-only (no video stream expected)
    bool isAudioOnly() const;
};
