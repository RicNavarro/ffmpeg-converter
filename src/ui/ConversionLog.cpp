#include "ConversionLog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QClipboard>
#include <QApplication>
#include <QDateTime>

ConversionLog::ConversionLog(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(4);

    // --- Header row: title + buttons ---
    auto *header = new QHBoxLayout();

    m_titleLabel = new QLabel("Log", this);
    QFont f = m_titleLabel->font();
    f.setPointSize(9);
    m_titleLabel->setFont(f);

    m_copyBtn  = new QPushButton("Copy", this);
    m_clearBtn = new QPushButton("Clear", this);
    m_copyBtn->setFixedHeight(22);
    m_clearBtn->setFixedHeight(22);
    m_copyBtn->setFlat(true);
    m_clearBtn->setFlat(true);

    header->addWidget(m_titleLabel);
    header->addStretch();
    header->addWidget(m_copyBtn);
    header->addWidget(m_clearBtn);
    root->addLayout(header);

    // --- Text area ---
    m_text = new QTextEdit(this);
    m_text->setReadOnly(true);
    m_text->setMinimumHeight(150);
    m_text->setFontFamily("Courier New");
    m_text->setFontPointSize(8);
    // Subtle background differentiation
    m_text->setStyleSheet(
        "QTextEdit { background: #111111; color: #cccccc; border: 1px solid #333; border-radius: 4px; }"
    );
    root->addWidget(m_text, 1);

    connect(m_copyBtn,  &QPushButton::clicked, this, &ConversionLog::onCopyClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &ConversionLog::onClearClicked);
}

// ---------------------------------------------------------------------------

void ConversionLog::appendLine(const QString &line)
{
    // Colour-code key lines for readability
    QString html;
    if (line.startsWith("✔")) {
        html = QString("<span style='color:#5cb85c;'>%1</span>").arg(line.toHtmlEscaped());
    } else if (line.startsWith("✘") || line.startsWith("Error")) {
        html = QString("<span style='color:#d9534f;'>%1</span>").arg(line.toHtmlEscaped());
    } else if (line.startsWith("$")) {
        html = QString("<span style='color:#5bc0de;'>%1</span>").arg(line.toHtmlEscaped());
    } else if (line.startsWith("---")) {
        html = QString("<span style='color:#555555;'>%1</span>").arg(line.toHtmlEscaped());
    } else if (line.contains("time=")) {
        // Progress lines — dimmed, they're verbose
        html = QString("<span style='color:#888888;font-size:8pt;'>%1</span>")
               .arg(line.toHtmlEscaped());
    } else {
        html = QString("<span>%1</span>").arg(line.toHtmlEscaped());
    }

    m_text->append(html);

    // Auto-scroll to bottom
    QScrollBar *sb = m_text->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void ConversionLog::clear()
{
    m_text->clear();
    const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    appendLine(QString("[%1] Log cleared.").arg(ts));
}

void ConversionLog::onCopyClicked()
{
    QApplication::clipboard()->setText(m_text->toPlainText());
}

void ConversionLog::onClearClicked()
{
    m_text->clear();
}
