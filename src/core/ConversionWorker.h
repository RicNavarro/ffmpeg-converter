#pragma once

#include "FormatProfile.h"
#include <QObject>
#include <QProcess>

class ConversionWorker : public QObject
{
    Q_OBJECT

public:
    explicit ConversionWorker(const FormatProfile &profile, QObject *parent = nullptr);

public slots:
    void run();     // Called from the worker QThread
    void cancel();  // Thread-safe cancel request

signals:
    void progressChanged(int percent);          // 0-100
    void logLine(const QString &line);          // raw ffmpeg stderr line
    void finished(bool success, const QString &errorMsg);

private:
    void parseLine(const QString &line);
    QString buildFFmpegPath() const;

    FormatProfile   m_profile;
    QProcess       *m_process = nullptr;
    bool            m_cancelled = false;
    qint64          m_durationMs = 0;   // total media duration, parsed from first stderr lines
};
