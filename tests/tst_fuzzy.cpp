#include "jot/Fuzzy.h"

#include <QTest>

using namespace jot;
using namespace Qt::StringLiterals;

class TestFuzzy : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void matchesInOrderIgnoringCase();
    void prefersWordStartsAndRuns_data();
    void prefersWordStartsAndRuns();
    void givesThePositionsMatched();
};

void TestFuzzy::matchesInOrderIgnoringCase()
{
    QVERIFY(fuzzyScore(u"mv", u"Mara Vey"));
    QVERIFY(fuzzyScore(u"MARA", u"mara"));
    QVERIFY(!fuzzyScore(u"vm", u"Mara Vey"));
    QVERIFY(!fuzzyScore(u"maras", u"mara"));
    QCOMPARE(fuzzyScore(u"", u"anything"), std::optional(0));
}

void TestFuzzy::prefersWordStartsAndRuns_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("better");
    QTest::addColumn<QString>("worse");
    QTest::newRow("word starts") << u"mv"_s << u"Mara Vey"_s << u"Lemon verbena"_s;
    QTest::newRow("a run") << u"ferry"_s << u"The Ferry"_s << u"Fear every reply"_s;
    QTest::newRow("shorter") << u"mara"_s << u"Mara"_s << u"Mara Vey and the river"_s;
    QTest::newRow("camel case") << u"tn"_s << u"TaskNotes"_s << u"Tension"_s;
    QTest::newRow("path segment") << u"cm"_s << u"Cast/Mara"_s << u"Accommodation"_s;
    QTest::newRow("earlier") << u"ch"_s << u"Chapter 1"_s << u"Rich chapter"_s;
}

void TestFuzzy::prefersWordStartsAndRuns()
{
    QFETCH(QString, pattern);
    QFETCH(QString, better);
    QFETCH(QString, worse);
    const std::optional<int> good = fuzzyScore(pattern, better);
    const std::optional<int> bad = fuzzyScore(pattern, worse);
    QVERIFY(good);
    QVERIFY(bad);
    QVERIFY2(*good > *bad, qPrintable(u"%1 vs %2"_s.arg(*good).arg(*bad)));
}

void TestFuzzy::givesThePositionsMatched()
{
    QList<qsizetype> positions;
    QVERIFY(fuzzyScore(u"mv", u"Mara Vey", &positions));
    QCOMPARE(positions, (QList<qsizetype>{0, 5}));
    QVERIFY(fuzzyScore(u"fer", u"The Ferry", &positions));
    QCOMPARE(positions, (QList<qsizetype>{4, 5, 6}));
}

QTEST_GUILESS_MAIN(TestFuzzy)
#include "tst_fuzzy.moc"
