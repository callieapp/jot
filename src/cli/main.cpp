#include <QCommandLineParser>
#include <QCoreApplication>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(u"jot"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(JOT_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Plain-file notes for Linux."_s);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    return 0;
}
