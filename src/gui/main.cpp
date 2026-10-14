#include "ThemeController.h"

#include "jot/Identity.h"
#include "jot/LogFile.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTextStream>

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

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption themeOption(
        u"theme"_s, u"Built-in theme id or path to a theme file. Defaults to $JOT_THEME."_s,
        u"theme"_s, qEnvironmentVariable("JOT_THEME"));
    parser.addOption(themeOption);
    parser.process(app);
    jot::logfile::install(u"jot-gui"_s);

    QTextStream err(stderr);
    if (const QString theme = parser.value(themeOption); !theme.isEmpty()) {
        const QStringList errors = jot::ThemeController::instance()->load(theme);
        for (const QString &error : errors)
            err << "jot-gui: " << theme << ": " << error << "\n";
        if (!errors.isEmpty())
            return 1;
    }
    for (const QString &warning : jot::ThemeController::instance()->warnings())
        err << "jot-gui: theme: " << warning << "\n";
    err.flush();

    // Basic style: everything visual comes from the theme, not a platform style.
    QQuickStyle::setStyle(u"Basic"_s);
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Jot.Ui", "Main");
    return app.exec();
}
