#include "ConversionWorker.h"
#include "FFmpegPresets.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ConversionWorker::ConversionWorker(const FormatProfile &profile, QObject *parent)
    : QObject(parent)
    , m_profile(profile)
{
}

// ---------------------------------------------------------------------------
// run() — executed inside the worker QThread
// ---------------------------------------------------------------------------

void ConversionWorker::run()
{
    const QString ffmpegBin = buildFFmpegPath();

    // On Windows we can check file existence of the bundled binary.
    // On Linux ffmpegBin is just "ffmpeg" (on PATH), so we probe it instead.
#ifdef Q_OS_WIN
    if (!QFileInfo::exists(ffmpegBin)) {
        emit finished(false, "ffmpeg.exe not found at: " + ffmpegBin);
        return;
    }
#else
    {
        QProcess probe;
        probe.start(ffmpegBin, { "-version" });
        if (!probe.waitForStarted(3000) || !probe.waitForFinished(3000)) {
            emit finished(false,
                "ffmpeg not found. Please install it:\n"
                "  sudo apt install ffmpeg");
            return;
        }
    }
#endif

    // Build argument list:
    //   ffmpeg -y -i <input> [codec/filter args...] <output>
    QStringList args;
    args << "-y"                          // overwrite output without asking
         << "-i" << m_profile.inputPath;
    args += FFmpegPresets::buildArgs(m_profile);
    args << m_profile.outputPath();

    // Log the command we are about to run
    emit logLine("$ ffmpeg " + args.join(' '));
    emit logLine("---");

    // Create and configure QProcess
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels); // stderr + stdout merged

    // Connect readyRead so we can stream output line by line
    connect(m_process, &QProcess::readyRead, this, [this]() {
        while (m_process->canReadLine()) {
            const QString line = QString::fromLocal8Bit(m_process->readLine()).trimmed();
            if (!line.isEmpty()) {
                emit logLine(line);
                parseLine(line);
            }
        }
    });

    m_process->start(ffmpegBin, args);

    if (!m_process->waitForStarted(5000)) {
        emit finished(false, "Failed to start ffmpeg process.");
        return;
    }

    // Block until done (this runs in a QThread, so the GUI stays responsive)
    m_process->waitForFinished(-1);

    // Flush any remaining output
    while (m_process->canReadLine()) {
        const QString line = QString::fromLocal8Bit(m_process->readLine()).trimmed();
        if (!line.isEmpty()) {
            emit logLine(line);
            parseLine(line);
        }
    }

    const int exitCode = m_process->exitCode();
    const QProcess::ExitStatus status = m_process->exitStatus();

    if (m_cancelled) {
        emit finished(false, "Cancelled by user.");
        return;
    }

    if (status == QProcess::NormalExit && exitCode == 0) {
        emit progressChanged(100);
        emit finished(true, {});
    } else {
        emit finished(false, QString("ffmpeg exited with code %1").arg(exitCode));
    }
}

// ---------------------------------------------------------------------------
// cancel() — can be called from any thread
// ---------------------------------------------------------------------------

void ConversionWorker::cancel()
{
    m_cancelled = true;
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
}

// ---------------------------------------------------------------------------
// parseLine() — extracts duration and current time to compute progress %
//
// ffmpeg stderr format examples:
//   Duration: 00:01:23.45, start: 0.000000, ...
//   frame=  142 fps= 48 q=28.0 size=    512kB time=00:00:05.92 bitrate= 708.4kbits/s speed=2.4x
// ---------------------------------------------------------------------------

static qint64 parseTimestamp(const QString &ts)
{
    // Parses "HH:MM:SS.xx" → milliseconds
    const QStringList parts = ts.split(':');
    if (parts.size() != 3) return 0;

    const int    h   = parts[0].toInt();
    const int    m   = parts[1].toInt();
    const double s   = parts[2].toDouble();

    return static_cast<qint64>((h * 3600 + m * 60 + s) * 1000.0);
}

void ConversionWorker::parseLine(const QString &line)
{
    // --- Capture total duration (only needed once) ---
    if (m_durationMs == 0) {
        static const QRegularExpression durationRe(
            R"(Duration:\s*(\d{2}:\d{2}:\d{2}\.\d+))"
        );
        const auto dm = durationRe.match(line);
        if (dm.hasMatch()) {
            m_durationMs = parseTimestamp(dm.captured(1));
        }
    }

    // --- Capture current encode time → compute percent ---
    if (m_durationMs > 0) {
        static const QRegularExpression timeRe(
            R"(time=(\d{2}:\d{2}:\d{2}\.\d+))"
        );
        const auto tm = timeRe.match(line);
        if (tm.hasMatch()) {
            const qint64 currentMs = parseTimestamp(tm.captured(1));
            const int percent = static_cast<int>(
                qMin(100LL, currentMs * 100LL / m_durationMs)
            );
            emit progressChanged(percent);
        }
    }
}

// ---------------------------------------------------------------------------
// buildFFmpegPath() — cross-platform ffmpeg discovery
//
// Windows: looks for a bundled ffmpeg.exe next to the application binary.
// Linux:   ffmpeg is a system package (apt install ffmpeg) and lives on PATH.
//          We still do a quick sanity-check via QProcess to give a clear error.
// ---------------------------------------------------------------------------

QString ConversionWorker::buildFFmpegPath() const
{
#ifdef Q_OS_WIN
    // Primary: <appDir>/ffmpeg/ffmpeg.exe  (bundled)
    const QString bundled = QDir(QCoreApplication::applicationDirPath())
                            .filePath("ffmpeg/ffmpeg.exe");
    if (QFileInfo::exists(bundled))
        return bundled;

    // Fallback: flat layout
    const QString flat = QDir(QCoreApplication::applicationDirPath())
                         .filePath("ffmpeg.exe");
    if (QFileInfo::exists(flat))
        return flat;
#endif

    // Linux / macOS: rely on system PATH
    return "ffmpeg";
}
