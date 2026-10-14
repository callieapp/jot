#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(u"jot"_s);
    QGuiApplication::setApplicationDisplayName(u"Jot"_s);
    QGuiApplication::setApplicationVersion(QStringLiteral(JOT_VERSION));
    QGuiApplication::setDesktopFileName(QStringLiteral(JOT_APP_ID));
    QQuickStyle::setStyle(u"Basic"_s);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Jot.Ui", "Main");
    return app.exec();
}
