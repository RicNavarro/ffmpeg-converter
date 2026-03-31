#include "DropZone.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QPainter>
#include <QSet>

// ---------------------------------------------------------------------------
// Supported input extensions
// ---------------------------------------------------------------------------

static bool isSupportedMedia(const QString &path)
{
    static const QSet<QString> exts = {
        // Video
        "mp4","mkv","avi","mov","wmv","flv","webm","m4v","3gp","ts","m2ts",
        // Audio
        "mp3","aac","flac","ogg","wav","m4a","opus","wma","aiff","alac",
        // Image sequences / GIF
        "gif"
    };
    return exts.contains(QFileInfo(path).suffix().toLower());
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

DropZone::DropZone(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setMinimumHeight(120);
    setCursor(Qt::PointingHandCursor);

    m_label = new QLabel("Drag & drop a file here\nor click to browse", this);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setWordWrap(true);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void DropZone::setFile(const QString &path)
{
    m_filePath = path;
    const QString name = QFileInfo(path).fileName();
    m_label->setText("📄  " + name + "\n\nClick to change file");
    update();
}

void DropZone::clear()
{
    m_filePath.clear();
    m_label->setText("Drag & drop a file here\nor click to browse");
    update();
}

// ---------------------------------------------------------------------------
// Drag & drop events
// ---------------------------------------------------------------------------

void DropZone::dragEnterEvent(QDragEnterEvent *event)
{
    const auto *mime = event->mimeData();
    if (mime->hasUrls() && mime->urls().size() == 1) {
        const QString path = mime->urls().first().toLocalFile();
        if (isSupportedMedia(path)) {
            setHovered(true);
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void DropZone::dragLeaveEvent(QDragLeaveEvent *event)
{
    setHovered(false);
    QWidget::dragLeaveEvent(event);
}

void DropZone::dropEvent(QDropEvent *event)
{
    setHovered(false);
    const QString path = event->mimeData()->urls().first().toLocalFile();
    if (isSupportedMedia(path)) {
        setFile(path);
        emit fileDropped(path);
        event->acceptProposedAction();
    }
}

// ---------------------------------------------------------------------------
// Click to browse
// ---------------------------------------------------------------------------

void DropZone::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select media file"),
        {},
        tr("Media files (*.mp4 *.mkv *.avi *.mov *.wmv *.flv *.webm *.m4v "
           "*.mp3 *.aac *.flac *.ogg *.wav *.m4a *.opus *.wma *.gif);;"
           "All files (*.*)")
    );
    if (!path.isEmpty()) {
        setFile(path);
        emit fileDropped(path);
    }
}

// ---------------------------------------------------------------------------
// Custom painting — dashed border, hover highlight
// ---------------------------------------------------------------------------

void DropZone::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor borderColor  = m_hovered ? QColor("#4A90D9") : QColor("#888888");
    const QColor bgColor      = m_hovered ? QColor("#1A3A5C")
                                          : (m_filePath.isEmpty() ? QColor("#1E1E1E") : QColor("#1A2A1E"));

    // Background
    p.setPen(Qt::NoPen);
    p.setBrush(bgColor);
    p.drawRoundedRect(rect(), 10, 10);

    // Dashed border
    QPen pen(borderColor, 2, Qt::DashLine);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 9, 9);
}

void DropZone::setHovered(bool on)
{
    m_hovered = on;
    update();
}
