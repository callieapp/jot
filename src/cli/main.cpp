#include "Completion.h"
#include "Paint.h"
#include "Palette.h"
#include "UpdateHint.h"
#include "Upgrade.h"

#include "jot/Diagnostics.h"
#include "jot/Identity.h"
#include "jot/LogFile.h"
#include "jot/UpdateCheck.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTextStream>

#include <optional>

#include <unistd.h>

using namespace jot;
using namespace Qt::StringLiterals;

namespace {

QTextStream out(stdout);
QTextStream err(stderr);

/// Colors from $JOT_THEME, or the default theme, when the output is a terminal.
const cli::Paint &paintFor(int fd)
{
    static const cli::Paint paint = [fd] {
        const cli::Paint::Depth depth = cli::Paint::detect(fd);
        if (depth == cli::Paint::Depth::None)
            return cli::Paint();
        return cli::Paint(depth, cli::Palette::forTheme(qEnvironmentVariable("JOT_THEME")));
    }();
    return paint;
}

/// The commands, for --help and for completion.
const QList<std::pair<QString, QString>> kCommands = {
    {u"logs"_s, u"Show, follow (-f) or open (--open) the log files"_s},
    {u"doctor"_s, u"Details for a bug report; --report opens a new issue"_s},
    {u"upgrade"_s, u"Install the latest release, or the one given: jot upgrade 0.1.0"_s},
    {u"downgrade"_s, u"Go back to the release before this one, or the one given"_s},
    {u"completions"_s, u"Print the shell completion script: bash, zsh or fish"_s},
    {u"gui"_s, u"Open the desktop app"_s},
};

/// After a command on a terminal, says once that a newer release is out. GitHub
/// is asked at most once a day, by a copy of jot left running on its own, so no
/// command waits for it.
class HintAfterwards
{
public:
    explicit HintAfterwards(QString command) : m_command(std::move(command)) {}
    HintAfterwards(const HintAfterwards &) = delete;
    HintAfterwards &operator=(const HintAfterwards &) = delete;

    ~HintAfterwards()
    {
        UpdateCheck check;
        const cli::UpdateHint hint = cli::UpdateHint::after(
            m_command, false, ::isatty(STDERR_FILENO), true, check,
            QVersionNumber::fromString(QCoreApplication::applicationVersion()));
        if (!hint.line.isEmpty())
            err << paintFor(STDERR_FILENO).dim(hint.line) << "\n" << Qt::flush;
        if (hint.ask) {
            check.markAsked();
            QProcess asker;
            asker.setProgram(QCoreApplication::applicationFilePath());
            asker.setArguments({u"update-check"_s});
            asker.setStandardOutputFile(QProcess::nullDevice());
            asker.setStandardErrorFile(QProcess::nullDevice());
            asker.startDetached();
        }
    }

private:
    QString m_command;
};

int runDoctor(bool report)
{
    const QString text = diagnostics::collect();
    out << text << "\n" << Qt::flush;
    if (report) {
        const QString url = diagnostics::issueUrl(text).toString(QUrl::FullyEncoded);
        // Over SSH or without a desktop nothing opens, so the link is always shown.
        if (QProcess::startDetached(u"xdg-open"_s, {url}))
            err << QObject::tr("Opening a new issue in your browser. If it does not open, visit:")
                << "\n";
        else
            err << QObject::tr("Could not open a browser. To report the bug, visit:") << "\n";
        err << url << "\n";
    }
    return 0;
}

/// The log files, one path per line, newest first; or follows or opens them.
int runLogs(bool follow, bool open)
{
    const QString directory = logfile::defaultDirectory();
    if (open)
        return QProcess::execute(u"xdg-open"_s, {directory}) == 0 ? 0 : 1;

    QDir dir(directory);
    const QStringList names =
        dir.entryList({u"*.log"_s, u"*.log.[0-9]"_s}, QDir::Files, QDir::Time);
    if (names.isEmpty()) {
        err << QObject::tr("No logs yet in %1").arg(directory) << "\n";
        return 0;
    }
    QStringList paths;
    for (const QString &name : names)
        paths.append(dir.filePath(name));
    if (follow) {
        out.flush();
        return QProcess::execute(u"tail"_s, QStringList{u"-F"_s} + paths);
    }
    for (const QString &path : std::as_const(paths))
        out << path << "\n";
    return 0;
}

int runCompletions(const QString &shell)
{
    const QHash<QString, QString> files = {
        {u"bash"_s, u"jot.bash"_s}, {u"zsh"_s, u"_jot"_s}, {u"fish"_s, u"jot.fish"_s}};
    QFile script(u":/completions/"_s + files.value(shell, u"none"_s));
    if (!script.open(QIODevice::ReadOnly)) {
        err << QObject::tr("usage: jot completions bash|zsh|fish") << "\n";
        return 2;
    }
    out << script.readAll();
    return 0;
}

} // namespace

/// Quiet on success. Each failure is one line on stderr, and any failure exits 1.
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(identity::dirName());
    app.setApplicationVersion(QStringLiteral(JOT_VERSION));

    QCommandLineParser parser;
    QString description = u"Jot: plain-file notes for Linux.\n\nCommands:"_s;
    for (const auto &[name, about] : kCommands)
        description += u"\n  "_s + name.leftJustified(12) + about;
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(u"command"_s, u"Command to run."_s);
    // Kept for completion too, since the parser does not list them.
    QList<QCommandLineOption> options;
    const auto addOption = [&parser, &options](const QCommandLineOption &option) {
        parser.addOption(option);
        options.append(option);
    };
    const QCommandLineOption followOption({u"f"_s, u"follow"_s},
                                          u"With logs: keep printing new lines."_s);
    addOption(followOption);
    const QCommandLineOption openOption(u"open"_s, u"With logs: open the folder."_s);
    addOption(openOption);
    const QCommandLineOption verboseOption(u"verbose"_s, u"Print progress as well as warnings."_s);
    addOption(verboseOption);
    const QCommandLineOption reportOption(u"report"_s, u"With doctor: open a new GitHub issue."_s);
    addOption(reportOption);
    const QCommandLineOption forceOption(
        u"force"_s, u"With upgrade: install the release even when it is the one installed."_s);
    addOption(forceOption);
    const QCommandLineOption dryRunOption(
        u"dry-run"_s, u"With upgrade and downgrade: say what would happen, and change nothing."_s);
    addOption(dryRunOption);

    // Run by the shell completion scripts with the words typed so far, which
    // are not parsed as options since they are still being typed.
    if (app.arguments().value(1) == u"__complete") {
        // The shells hand over the line as typed, up to the cursor.
        const QStringList words = app.arguments().value(2) == u"--line"
                                      ? cli::Completion::wordsOf(app.arguments().value(3))
                                      : app.arguments().mid(2);
        QList<cli::Candidate> commands;
        for (const auto &[name, about] : kCommands)
            commands.append({name, about});
        QList<cli::Completion::Option> named;
        for (const QCommandLineOption &option : std::as_const(options)) {
            for (const QString &name : option.names())
                named.append({name, option.description(), !option.valueName().isEmpty()});
        }
        cli::Completion::Sources sources;
        sources.arguments = [](const QStringList &positional, const QString &) {
            if (positional == QStringList{u"completions"_s})
                return cli::Completion::plain({u"bash"_s, u"zsh"_s, u"fish"_s});
            return QList<cli::Candidate>();
        };
        const cli::Completion completion(commands, named, sources);
        for (const cli::Candidate &candidate : completion.complete(words))
            out << candidate.value
                << (candidate.about.isEmpty() ? QString() : u'\t' + candidate.about) << "\n";
        return 0;
    }

    parser.process(app);
    logfile::install(u"jot"_s);
    logfile::setVerboseTerminal(parser.isSet(verboseOption) ||
                                !qEnvironmentVariableIsEmpty("QT_LOGGING_RULES"));

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty())
        parser.showHelp(0);
    const QString command = args.first();

    // Run by HintAfterwards, on its own, to learn the latest release.
    if (command == u"update-check" && args.size() == 1) {
        UpdateCheck check;
        if (!check.wanted(true))
            return 0;
        QObject::connect(&check, &UpdateCheck::checked, &app, &QCoreApplication::quit);
        check.check();
        return app.exec();
    }

    const HintAfterwards hint(command);

    if (command == u"completions" && args.size() == 2)
        return runCompletions(args.at(1));
    if (command == u"doctor" && args.size() == 1)
        return runDoctor(parser.isSet(reportOption));
    if (command == u"logs" && args.size() == 1)
        return runLogs(parser.isSet(followOption), parser.isSet(openOption));
    if ((command == u"upgrade" || command == u"downgrade") && args.size() <= 2) {
        cli::Upgrade upgrade(out, err);
        cli::Upgrade::Options upgradeOptions;
        upgradeOptions.downgrade = command == u"downgrade";
        if (args.size() == 2)
            upgradeOptions.version = args.at(1);
        upgradeOptions.force = parser.isSet(forceOption);
        upgradeOptions.dryRun = parser.isSet(dryRunOption);
        std::optional<int> result;
        upgrade.run(upgradeOptions, [&result](int code) {
            result = code;
            QCoreApplication::exit(code);
        });
        return result ? *result : app.exec();
    }
    if (command == u"gui" && args.size() == 1)
        return QProcess::startDetached(u"jot-gui"_s, {}) ? 0 : 1;

    err << QObject::tr("jot: unknown command '%1'").arg(args.join(u' ')) << "\n";
    parser.showHelp(1);
}
