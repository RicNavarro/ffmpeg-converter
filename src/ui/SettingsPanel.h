#pragma once

#include "../core/FormatProfile.h"
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);

    // Reads current widget values into a partially-filled profile.
    // (inputPath and outputDir are set by MainWindow)
    void applyTo(FormatProfile &profile) const;

    QString selectedFormat() const;

signals:
    void formatChanged(const QString &format);

private slots:
    void onFormatChanged(int index);

private:
    void populateCodecs(const QString &format);

    QComboBox *m_formatCombo;
    QComboBox *m_videoCodecCombo;
    QComboBox *m_audioCodecCombo;
    QComboBox *m_resolutionCombo;
    QSpinBox  *m_videoBitrateSpin;
    QSpinBox  *m_audioBitrateSpin;
    QGroupBox *m_advancedGroup;
};
