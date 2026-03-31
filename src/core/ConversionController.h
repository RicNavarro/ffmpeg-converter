#pragma once

#include "FormatProfile.h"
#include <QObject>
#include <QQueue>
#include <QThread>

class ConversionWorker;

class ConversionController : public QObject
{
    Q_OBJECT

public:
    explicit ConversionController(QObject *parent = nullptr);
    ~ConversionController();

    void enqueue(const FormatProfile &profile);
    void cancelCurrent();
    bool isBusy() const;

signals:
    void jobStarted(const QString &inputPath);
    void progressChanged(int percent);
    void logLine(const QString &line);
    void jobFinished(bool success, const QString &errorMsg);
    void allJobsDone();

private slots:
    void onJobFinished(bool success, const QString &errorMsg);

private:
    void startNextJob();
    void cleanup();

    QQueue<FormatProfile>  m_queue;
    QThread               *m_thread = nullptr;
    ConversionWorker      *m_worker = nullptr;
};
