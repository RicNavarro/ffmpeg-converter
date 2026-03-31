#include "AppSettings.h"

#include <QSettings>
#include <QStandardPaths>

// Helper — creates a QSettings instance pointing to our app's registry key
static QSettings cfg()
{
    return QSettings("FFmpegWrapper", "FFmpegConverter");
}

// ---------------------------------------------------------------------------

QString AppSettings::outputDir()
{
    auto s = cfg();
    return s.value(
        "outputDir",
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
    ).toString();
}

void AppSettings::setOutputDir(const QString &dir)
{
    auto s = cfg();
    s.setValue("outputDir", dir);
}

int AppSettings::lastFormatIndex()
{
    return cfg().value("lastFormatIndex", 0).toInt();
}

void AppSettings::setLastFormatIndex(int index)
{
    auto s = cfg();
    s.setValue("lastFormatIndex", index);
}

bool AppSettings::advancedExpanded()
{
    return cfg().value("advancedExpanded", false).toBool();
}

void AppSettings::setAdvancedExpanded(bool expanded)
{
    auto s = cfg();
    s.setValue("advancedExpanded", expanded);
}

int AppSettings::videoBitrate()
{
    return cfg().value("videoBitrate", 0).toInt();
}

void AppSettings::setVideoBitrate(int kbps)
{
    auto s = cfg();
    s.setValue("videoBitrate", kbps);
}

int AppSettings::audioBitrate()
{
    return cfg().value("audioBitrate", 0).toInt();
}

void AppSettings::setAudioBitrate(int kbps)
{
    auto s = cfg();
    s.setValue("audioBitrate", kbps);
}

QByteArray AppSettings::windowGeometry()
{
    return cfg().value("windowGeometry").toByteArray();
}

void AppSettings::setWindowGeometry(const QByteArray &geometry)
{
    auto s = cfg();
    s.setValue("windowGeometry", geometry);
}
