#include "MainWindow.h"
#include "DropZone.h"
#include "SettingsPanel.h"
#include "ConversionLog.h"
#include "../core/ConversionController.h"
#include "../core/FormatProfile.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

// ---------------------------------------------------------------------------
// Constructor — builds the entire UI
// ---------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_controller(new ConversionController(this))
{
    setWindowTitle("FFmpeg Converter");
    setMinimumSize(680, 700);

    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *root = new QVBoxLayout(central);
    root->setSpacing(12);
    root->setContentsMargins(20, 20, 20, 20);

    // --- Drop zone ---
    m_dropZone = new DropZone(central);
    root->addWidget(m_dropZone);

    // --- Settings panel ---
    m_settingsPanel = new SettingsPanel(central);
    root->addWidget(m_settingsPanel);

    // --- Output folder row ---
    auto *outRow = new QHBoxLayout();
    outRow->addWidget(new QLabel("Output folder:", central));
    m_outputDirEdit = new QLineEdit(
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        central
    );
    m_outputDirEdit->setReadOnly(true);
    outRow->addWidget(m_outputDirEdit, 1);
    m_browseOutputBtn = new QPushButton("Browse…", central);
    outRow->addWidget(m_browseOutputBtn);
    root->addLayout(outRow);

    // --- Convert / Cancel buttons ---
    auto *btnRow = new QHBoxLayout();
    m_convertBtn = new QPushButton("Convert", central);
    m_convertBtn->setEnabled(false);
    m_convertBtn->setFixedHeight(36);
    m_cancelBtn  = new QPushButton("Cancel", central);
    m_cancelBtn->setEnabled(false);
    m_cancelBtn->setFixedHeight(36);
    btnRow->addWidget(m_convertBtn, 3);
    btnRow->addWidget(m_cancelBtn,  1);
    root->addLayout(btnRow);

    // --- Progress bar ---
    m_progressBar = new QProgressBar(central);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    root->addWidget(m_progressBar);

    // --- Status label ---
    m_statusLabel = new QLabel("Ready.", central);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_statusLabel);

    // --- Log view ---
    m_logView = new ConversionLog(central);
    root->addWidget(m_logView, 1);

    // -----------------------------------------------------------------------
    // Signal wiring
    // -----------------------------------------------------------------------

    connect(m_dropZone,        &DropZone::fileDropped,
            this,              &MainWindow::onFileDropped);

    connect(m_browseOutputBtn, &QPushButton::clicked,
            this,              &MainWindow::onBrowseOutput);

    connect(m_convertBtn,      &QPushButton::clicked,
            this,              &MainWindow::onConvertClicked);

    connect(m_cancelBtn,       &QPushButton::clicked,
            this,              &MainWindow::onCancelClicked);

    connect(m_controller, &ConversionController::jobStarted,
            this,         &MainWindow::onJobStarted);

    connect(m_controller, &ConversionController::progressChanged,
            this,         &MainWindow::onProgressChanged);

    connect(m_controller, &ConversionController::logLine,
            this,         &MainWindow::onLogLine);

    connect(m_controller, &ConversionController::jobFinished,
            this,         &MainWindow::onJobFinished);

    connect(m_controller, &ConversionController::allJobsDone,
            this,         &MainWindow::onAllJobsDone);
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void MainWindow::onFileDropped(const QString &path)
{
    Q_UNUSED(path)
    m_convertBtn->setEnabled(true);
    m_statusLabel->setText("File loaded. Ready to convert.");
}

void MainWindow::onBrowseOutput()
{
    const QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select output folder"),
        m_outputDirEdit->text()
    );
    if (!dir.isEmpty())
        m_outputDirEdit->setText(dir);
}

void MainWindow::onConvertClicked()
{
    const QString input = m_dropZone->currentFile();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "No file", "Please select a file first.");
        return;
    }

    const QString outDir = m_outputDirEdit->text();
    if (outDir.isEmpty() || !QDir(outDir).exists()) {
        QMessageBox::warning(this, "Invalid folder", "Please select a valid output folder.");
        return;
    }

    FormatProfile profile;
    profile.inputPath = input;
    profile.outputDir = outDir;
    m_settingsPanel->applyTo(profile);

    m_logView->clear();
    m_progressBar->setValue(0);
    setUiBusy(true);

    m_controller->enqueue(profile);
}

void MainWindow::onCancelClicked()
{
    m_controller->cancelCurrent();
    m_statusLabel->setText("Cancelling…");
}

void MainWindow::onJobStarted(const QString &path)
{
    const QString name = path.section('/', -1);
    m_statusLabel->setText("Converting: " + name);
    m_logView->appendLine(QString("[%1] Starting: %2")
        .arg(QDateTime::currentDateTime().toString("HH:mm:ss"), name));
}

void MainWindow::onProgressChanged(int percent)
{
    m_progressBar->setValue(percent);
}

void MainWindow::onLogLine(const QString &line)
{
    m_logView->appendLine(line);
}

void MainWindow::onJobFinished(bool success, const QString &errorMsg)
{
    if (success) {
        m_logView->appendLine("✔  Conversion complete.");
    } else {
        m_logView->appendLine("✘  Error: " + errorMsg);
        QMessageBox::warning(this, "Conversion failed", errorMsg);
    }
}

void MainWindow::onAllJobsDone()
{
    setUiBusy(false);
    m_progressBar->setValue(100);
    m_statusLabel->setText("Done! All conversions finished.");
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void MainWindow::setUiBusy(bool busy)
{
    m_convertBtn->setEnabled(!busy);
    m_cancelBtn->setEnabled(busy);
    m_dropZone->setEnabled(!busy);
    m_settingsPanel->setEnabled(!busy);
    m_browseOutputBtn->setEnabled(!busy);
}
