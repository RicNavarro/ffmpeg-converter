#pragma once

#include "FormatProfile.h"
#include <QString>
#include <QStringList>
#include <QMap>

// Describes one selectable output format shown in the UI
struct FormatOption {
    QString extension;      // "mp4"
    QString label;          // "MP4 – H.264/AAC (universal)"
    QString category;       // "Video", "Audio", "Image"
};

class FFmpegPresets
{
public:
    // Returns all formats available in the UI, ordered by category
    static QList<FormatOption> availableFormats();

    // Returns codec choices for a given output format
    // (populates the "Video codec" combo box in SettingsPanel)
    static QStringList videoCodecsFor(const QString &format);
    static QStringList audioCodecsFor(const QString &format);

    // Builds the complete ffmpeg argument list for a given profile.
    // Does NOT include -i <input> or the output path — those are
    // added by ConversionWorker.
    static QStringList buildArgs(const FormatProfile &profile);

private:
    // Returns the baseline arg list for a format before user overrides
    static QStringList baseArgs(const QString &format,
                                const QString &videoCodec,
                                const QString &audioCodec);
};
