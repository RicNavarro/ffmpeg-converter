#pragma once

#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class DropZone;
class SettingsPanel;
class ConversionLog;
class ConversionController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onFileDropped(const QString &path);
    void onBrowseOutput();
    void onConvertClicked();
    void onCancelClicked();
    void onJobStarted(const QString &path);
    void onProgressChanged(int percent);
    void onLogLine(const QString &line);
    void onJobFinished(bool success, const QString &errorMsg);
    void onAllJobsDone();

private:
    void setUiBusy(bool busy);

    // Widgets
    DropZone            *m_dropZone;
    SettingsPanel       *m_settingsPanel;
    QLineEdit           *m_outputDirEdit;
    QPushButton         *m_browseOutputBtn;
    QPushButton         *m_convertBtn;
    QPushButton         *m_cancelBtn;
    QProgressBar        *m_progressBar;
    ConversionLog       *m_logView;
    QLabel              *m_statusLabel;

    // Logic
    ConversionController *m_controller;
};
