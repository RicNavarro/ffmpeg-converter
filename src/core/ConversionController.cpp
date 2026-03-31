#include "ConversionController.h"
#include "ConversionWorker.h"

ConversionController::ConversionController(QObject *parent)
    : QObject(parent)
{
}

ConversionController::~ConversionController()
{
    cancelCurrent();
}

void ConversionController::enqueue(const FormatProfile &profile)
{
    m_queue.enqueue(profile);
    if (!isBusy())
        startNextJob();
}

void ConversionController::cancelCurrent()
{
    if (m_worker)
        m_worker->cancel();
}

bool ConversionController::isBusy() const
{
    return m_thread && m_thread->isRunning();
}

void ConversionController::startNextJob()
{
    if (m_queue.isEmpty())
        return;

    const FormatProfile profile = m_queue.dequeue();
    emit jobStarted(profile.inputPath);

    // Create a fresh thread and worker for each job
    m_thread = new QThread(this);
    m_worker = new ConversionWorker(profile);
    m_worker->moveToThread(m_thread);

    // Wire up signals
    connect(m_thread, &QThread::started,        m_worker, &ConversionWorker::run);
    connect(m_worker, &ConversionWorker::progressChanged, this, &ConversionController::progressChanged);
    connect(m_worker, &ConversionWorker::logLine,         this, &ConversionController::logLine);
    connect(m_worker, &ConversionWorker::finished,        this, &ConversionController::onJobFinished);

    m_thread->start();
}

void ConversionController::onJobFinished(bool success, const QString &errorMsg)
{
    emit jobFinished(success, errorMsg);

    cleanup();

    if (!m_queue.isEmpty())
        startNextJob();
    else
        emit allJobsDone();
}

void ConversionController::cleanup()
{
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_worker;
        delete m_thread;
        m_worker = nullptr;
        m_thread = nullptr;
    }
}
