#include "SettingsPanel.h"
#include "../core/FFmpegPresets.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // --- Output format ---
    m_formatCombo = new QComboBox(this);
    for (const auto &opt : FFmpegPresets::availableFormats())
        m_formatCombo->addItem(opt.label, opt.extension);

    mainLayout->addWidget(new QLabel("Output format:", this));
    mainLayout->addWidget(m_formatCombo);

    // --- Advanced settings (collapsible group) ---
    m_advancedGroup = new QGroupBox("Advanced settings", this);
    m_advancedGroup->setCheckable(true);
    m_advancedGroup->setChecked(false);     // collapsed by default

    auto *advLayout = new QFormLayout(m_advancedGroup);

    m_videoCodecCombo = new QComboBox(this);
    m_audioCodecCombo = new QComboBox(this);

    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->addItems({ "Original", "3840x2160", "1920x1080", "1280x720", "854x480", "640x360" });

    m_videoBitrateSpin = new QSpinBox(this);
    m_videoBitrateSpin->setRange(0, 100000);
    m_videoBitrateSpin->setSuffix(" kbps");
    m_videoBitrateSpin->setSpecialValueText("Auto");
    m_videoBitrateSpin->setValue(0);

    m_audioBitrateSpin = new QSpinBox(this);
    m_audioBitrateSpin->setRange(0, 1411);
    m_audioBitrateSpin->setSuffix(" kbps");
    m_audioBitrateSpin->setSpecialValueText("Auto");
    m_audioBitrateSpin->setValue(0);

    advLayout->addRow("Video codec:",   m_videoCodecCombo);
    advLayout->addRow("Audio codec:",   m_audioCodecCombo);
    advLayout->addRow("Resolution:",    m_resolutionCombo);
    advLayout->addRow("Video bitrate:", m_videoBitrateSpin);
    advLayout->addRow("Audio bitrate:", m_audioBitrateSpin);

    mainLayout->addWidget(m_advancedGroup);

    // Populate codecs for the initial format
    onFormatChanged(0);

    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onFormatChanged);
}

// ---------------------------------------------------------------------------

void SettingsPanel::onFormatChanged(int index)
{
    const QString fmt = m_formatCombo->itemData(index).toString();
    populateCodecs(fmt);
    emit formatChanged(fmt);
}

void SettingsPanel::populateCodecs(const QString &format)
{
    m_videoCodecCombo->clear();
    m_audioCodecCombo->clear();

    const QStringList vCodecs = FFmpegPresets::videoCodecsFor(format);
    const QStringList aCodecs = FFmpegPresets::audioCodecsFor(format);

    if (vCodecs.isEmpty()) {
        m_videoCodecCombo->addItem("N/A (audio only)");
        m_videoCodecCombo->setEnabled(false);
    } else {
        m_videoCodecCombo->addItems(vCodecs);
        m_videoCodecCombo->setEnabled(true);
    }

    if (aCodecs.isEmpty()) {
        m_audioCodecCombo->addItem("N/A (video only)");
        m_audioCodecCombo->setEnabled(false);
    } else {
        m_audioCodecCombo->addItems(aCodecs);
        m_audioCodecCombo->setEnabled(true);
    }
}

// ---------------------------------------------------------------------------

QString SettingsPanel::selectedFormat() const
{
    return m_formatCombo->currentData().toString();
}

void SettingsPanel::applyTo(FormatProfile &profile) const
{
    profile.outputFormat = selectedFormat();

    if (m_videoCodecCombo->isEnabled())
        profile.videoCodec = m_videoCodecCombo->currentText();

    if (m_audioCodecCombo->isEnabled())
        profile.audioCodec = m_audioCodecCombo->currentText();

    if (m_resolutionCombo->currentIndex() > 0)    // index 0 = "Original"
        profile.resolution = m_resolutionCombo->currentText();

    if (m_videoBitrateSpin->value() > 0)
        profile.videoBitrate = m_videoBitrateSpin->value();

    if (m_audioBitrateSpin->value() > 0)
        profile.audioBitrate = m_audioBitrateSpin->value();
}
