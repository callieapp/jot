#include "jot/Identity.h"
#include "jot/LogFile.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    // The identity's folder name keeps Qt's own config and data folders apart for a
    // development build, and matches the CLI's.
    app.setApplicationName(jot::identity::dirName());
    app.setApplicationDisplayName(jot::identity::displayName());
    app.setApplicationVersion(QStringLiteral(JOT_VERSION));
    // Lets Wayland associate the window with the .desktop entry.
    app.setDesktopFileName(jot::identity::appId());
    QQuickStyle::setStyle(u"Basic"_s);
    jot::logfile::install(u"jot-gui"_s);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Jot.Ui", "Main");
    return app.exec();
}
