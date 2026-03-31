#include "FormatProfile.h"

#include <QFileInfo>
#include <QDir>
#include <QSet>

QString FormatProfile::outputPath() const
{
    const QString baseName = QFileInfo(inputPath).completeBaseName();
    return QDir(outputDir).filePath(baseName + "." + outputFormat);
}

bool FormatProfile::isAudioOnly() const
{
    static const QSet<QString> audioFormats = {
        "mp3", "aac", "flac", "ogg", "wav", "m4a", "opus", "wma", "aiff"
    };
    return audioFormats.contains(outputFormat.toLower());
}
