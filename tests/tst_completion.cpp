#include "Completion.h"

#include <QTest>

using namespace jot::cli;
using namespace Qt::StringLiterals;

class TestCompletion : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void commandsFirst();
    void optionsAndTheirValues();
    void argumentsComeFromTheApp();
    void linesAreReadAsTheShellReadsThem();
    void valuesAfterAnEqualsSignOrAShortOption();
};

namespace {

QStringList values(const QList<Candidate> &candidates)
{
    QStringList result;
    for (const Candidate &candidate : candidates)
        result.append(candidate.value);
    return result;
}

Completion make()
{
    Completion::Sources sources;
    sources.arguments = [](const QStringList &positional, const QString &) {
        if (positional == QStringList{u"completions"_s})
            return Completion::plain({u"bash"_s, u"zsh"_s, u"fish"_s});
        return QList<Candidate>();
    };
    sources.values = [](const QString &option) {
        return option == u"theme" ? Completion::plain({u"callie"_s, u"paper"_s})
                                  : QList<Candidate>();
    };
    return Completion({{u"doctor"_s, u"Details for a bug report"_s},
                       {u"downgrade"_s, u"Go back a release"_s},
                       {u"logs"_s, u"Show the log files"_s},
                       {u"completions"_s, u"Print a completion script"_s}},
                      {{u"follow"_s, u"Keep printing"_s, false},
                       {u"f"_s, u"Keep printing"_s, false},
                       {u"theme"_s, u"Colors"_s, true},
                       {u"t"_s, u"Colors"_s, true}},
                      sources);
}

} // namespace

void TestCompletion::commandsFirst()
{
    const Completion completion = make();
    QCOMPARE(values(completion.complete({u"do"_s})), (QStringList{u"doctor"_s, u"downgrade"_s}));
    QCOMPARE(completion.complete({u"lo"_s}).first().about, u"Show the log files"_s);
    // Options and their values before the command are skipped over.
    QCOMPARE(values(completion.complete({u"--theme"_s, u"paper"_s, u"lo"_s})),
             QStringList{u"logs"_s});
}

void TestCompletion::optionsAndTheirValues()
{
    const Completion completion = make();
    // Letters are left to the long names.
    QCOMPARE(values(completion.complete({u"logs"_s, u"--f"_s})), QStringList{u"--follow"_s});
    QCOMPARE(values(completion.complete({u"logs"_s, u"--theme"_s, QString()})),
             (QStringList{u"callie"_s, u"paper"_s}));
    QCOMPARE(values(completion.complete({u"logs"_s, u"--follow"_s, QString()})), QStringList());
}

void TestCompletion::argumentsComeFromTheApp()
{
    const Completion completion = make();
    QCOMPARE(values(completion.complete({u"completions"_s, u"z"_s})), QStringList{u"zsh"_s});
    QCOMPARE(values(completion.complete({u"completions"_s, u"zsh"_s, QString()})), QStringList());
    QCOMPARE(values(completion.complete({u"logs"_s, QString()})), QStringList());
}

void TestCompletion::linesAreReadAsTheShellReadsThem()
{
    QCOMPARE(Completion::wordsOf(u"jot logs "_s), (QStringList{u"logs"_s, QString()}));
    QCOMPARE(Completion::wordsOf(u"jot open My\\ No"_s), (QStringList{u"open"_s, u"My No"_s}));
    QCOMPARE(Completion::wordsOf(u"jot new 'Lunch at 1' --theme \"pa"_s),
             (QStringList{u"new"_s, u"Lunch at 1"_s, u"--theme"_s, u"pa"_s}));
}

void TestCompletion::valuesAfterAnEqualsSignOrAShortOption()
{
    const Completion completion = make();
    QCOMPARE(values(completion.complete({u"logs"_s, u"--theme=p"_s})),
             QStringList{u"--theme=paper"_s});
    QCOMPARE(values(completion.complete({u"--theme=paper"_s, u"lo"_s})), QStringList{u"logs"_s});
    QCOMPARE(values(completion.complete({u"-t"_s, u"paper"_s, u"lo"_s})), QStringList{u"logs"_s});
}

QTEST_GUILESS_MAIN(TestCompletion)
#include "tst_completion.moc"
