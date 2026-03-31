#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class ConversionLog : public QWidget
{
    Q_OBJECT

public:
    explicit ConversionLog(QWidget *parent = nullptr);

    void appendLine(const QString &line);
    void clear();

private slots:
    void onCopyClicked();
    void onClearClicked();

private:
    QTextEdit   *m_text;
    QPushButton *m_copyBtn;
    QPushButton *m_clearBtn;
    QLabel      *m_titleLabel;
};
