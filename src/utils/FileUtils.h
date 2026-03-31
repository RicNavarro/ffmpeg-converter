#pragma once

#include <QString>

// ---------------------------------------------------------------------------
// FileUtils
// Small collection of static file-handling helpers used across the app.
// ---------------------------------------------------------------------------

class FileUtils
{
public:
    // If 'path' doesn't exist, returns 'path' unchanged.
    // If it does exist, appends a numeric suffix until a free slot is found:
    //   video.mp4 → video (1).mp4 → video (2).mp4 → …
    static QString resolveConflict(const QString &path);

    // Opens the system file manager (Explorer on Windows) with 'path'
    // selected/highlighted inside its parent folder.
    static void showInExplorer(const QString &path);

    // Returns a human-readable file size string: "3.2 MB", "780 KB", etc.
    static QString humanSize(qint64 bytes);

private:
    FileUtils() = delete;
};
