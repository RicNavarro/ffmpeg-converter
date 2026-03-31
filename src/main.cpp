#include "ui/MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FFmpeg Converter");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("FFmpegWrapper");

#ifdef Q_OS_WIN
    // Force a dark Fusion palette on Windows.
    // On Linux we respect the system/desktop theme (GNOME, KDE, etc.)
    // so the app looks native on Pop!OS, Ubuntu, etc.
    app.setStyle("Fusion");
    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(30,  30,  30));
    dark.setColor(QPalette::WindowText,      Qt::white);
    dark.setColor(QPalette::Base,            QColor(18,  18,  18));
    dark.setColor(QPalette::AlternateBase,   QColor(40,  40,  40));
    dark.setColor(QPalette::ToolTipBase,     Qt::black);
    dark.setColor(QPalette::ToolTipText,     Qt::white);
    dark.setColor(QPalette::Text,            Qt::white);
    dark.setColor(QPalette::Button,          QColor(50,  50,  50));
    dark.setColor(QPalette::ButtonText,      Qt::white);
    dark.setColor(QPalette::BrightText,      Qt::red);
    dark.setColor(QPalette::Highlight,       QColor(74, 144, 217));
    dark.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(dark);
#endif

    MainWindow w;
    w.show();

    return app.exec();
}
