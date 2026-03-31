#pragma once

#include <QString>

// ---------------------------------------------------------------------------
// AppSettings
// Thin wrapper around QSettings that persists user preferences between
// sessions. All methods are static — no instantiation needed.
//
// Keys are written to the Windows registry under:
//   HKEY_CURRENT_USER\Software\FFmpegWrapper\FFmpegConverter
// ---------------------------------------------------------------------------

class AppSettings
{
public:
    // Last used output directory
    static QString outputDir();
    static void    setOutputDir(const QString &dir);

    // Last selected format index (combo box position)
    static int  lastFormatIndex();
    static void setLastFormatIndex(int index);

    // Advanced panel open/closed state
    static bool  advancedExpanded();
    static void  setAdvancedExpanded(bool expanded);

    // Last used video bitrate (0 = auto)
    static int  videoBitrate();
    static void setVideoBitrate(int kbps);

    // Last used audio bitrate (0 = auto)
    static int  audioBitrate();
    static void setAudioBitrate(int kbps);

    // Window geometry
    static QByteArray windowGeometry();
    static void       setWindowGeometry(const QByteArray &geometry);

private:
    AppSettings() = delete;
};
