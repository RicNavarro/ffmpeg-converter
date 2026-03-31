#pragma once

#include <QWidget>
#include <QLabel>

class DropZone : public QWidget
{
    Q_OBJECT

public:
    explicit DropZone(QWidget *parent = nullptr);

    QString currentFile() const { return m_filePath; }
    void    setFile(const QString &path);
    void    clear();

signals:
    void fileDropped(const QString &path);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event)           override;
    void mousePressEvent(QMouseEvent *event)    override;
    void paintEvent(QPaintEvent *event)         override;

private:
    bool    isMediaFile(const QString &path) const;
    void    setHovered(bool on);

    QLabel *m_label;
    QString m_filePath;
    bool    m_hovered = false;
};
