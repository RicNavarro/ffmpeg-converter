#include "FileUtils.h"

#include <QFileInfo>
#include <QDir>
#include <QProcess>

// ---------------------------------------------------------------------------

QString FileUtils::resolveConflict(const QString &path)
{
    if (!QFileInfo::exists(path))
        return path;

    const QFileInfo fi(path);
    const QString dir      = fi.absolutePath();
    const QString base     = fi.completeBaseName();
    const QString ext      = fi.suffix();
    const QString dotExt   = ext.isEmpty() ? QString() : ("." + ext);

    int counter = 1;
    QString candidate;
    do {
        candidate = QDir(dir).filePath(
            QString("%1 (%2)%3").arg(base).arg(counter).arg(dotExt)
        );
        ++counter;
    } while (QFileInfo::exists(candidate) && counter < 10000);

    return candidate;
}

// ---------------------------------------------------------------------------

void FileUtils::showInExplorer(const QString &path)
{
#ifdef Q_OS_WIN
    // Windows Explorer: opens the folder with the file selected
    const QString nativePath = QDir::toNativeSeparators(path);
    QProcess::startDetached("explorer.exe", { "/select,", nativePath });

#elif defined(Q_OS_LINUX)
    // On Linux we open the *parent folder* via xdg-open.
    // Most file managers (Nautilus, Thunar, Dolphin, Nemo) will open it.
    // There is no universal "select file" equivalent across all DEs.
    const QString parentDir = QFileInfo(path).absolutePath();
    QProcess::startDetached("xdg-open", { parentDir });

#elif defined(Q_OS_MACOS)
    // macOS Finder: reveal and select the file
    QProcess::startDetached("open", { "-R", path });
#endif
}

// ---------------------------------------------------------------------------

QString FileUtils::humanSize(qint64 bytes)
{
    if (bytes < 0)
        return "—";

    constexpr qint64 KB = 1024;
    constexpr qint64 MB = 1024 * KB;
    constexpr qint64 GB = 1024 * MB;

    if (bytes >= GB)
        return QString::number(double(bytes) / GB, 'f', 2) + " GB";
    if (bytes >= MB)
        return QString::number(double(bytes) / MB, 'f', 1) + " MB";
    if (bytes >= KB)
        return QString::number(double(bytes) / KB, 'f', 0) + " KB";

    return QString::number(bytes) + " B";
}
