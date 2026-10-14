#include "jot/Identity.h"
#include "jot/LogFile.h"

#include <QCommandLineParser>
#include <QCoreApplication>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(jot::identity::dirName());
    QCoreApplication::setApplicationVersion(QStringLiteral(JOT_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Plain-file notes for Linux."_s);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption verboseOption(u"verbose"_s, u"Show progress messages."_s);
    parser.addOption(verboseOption);
    parser.process(app);
    jot::logfile::install(u"jot"_s);
    jot::logfile::setVerboseTerminal(parser.isSet(verboseOption) ||
                                     !qEnvironmentVariableIsEmpty("QT_LOGGING_RULES"));
    return 0;
}
