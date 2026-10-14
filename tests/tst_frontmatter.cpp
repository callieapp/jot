#include "jot/FrontMatter.h"

#include <QTest>

using namespace Qt::StringLiterals;

class TestFrontMatter : public QObject
{
    Q_OBJECT

private slots:
    void splits_data();
    void splits();
};

void TestFrontMatter::splits_data()
{
    QTest::addColumn<QString>("note");
    QTest::addColumn<bool>("has");
    QTest::addColumn<QString>("yaml");
    QTest::addColumn<QString>("body");

    QTest::newRow("none") << u"# Title\nText\n"_s << false << QString() << u"# Title\nText\n"_s;
    QTest::newRow("simple") << u"---\ntags: [a]\n---\nBody\n"_s << true << u"tags: [a]\n"_s
                            << u"Body\n"_s;
    QTest::newRow("dots close") << u"---\na: 1\n...\nBody"_s << true << u"a: 1\n"_s << u"Body"_s;
    QTest::newRow("crlf") << u"---\r\na: 1\r\n---\r\nBody"_s << true << u"a: 1\r\n"_s << u"Body"_s;
    QTest::newRow("empty") << u"---\n---\nBody"_s << true << QString() << u"Body"_s;
    QTest::newRow("at end") << u"---\na: 1\n---"_s << true << u"a: 1\n"_s << QString();
    QTest::newRow("unclosed") << u"---\na: 1\nBody"_s << false << QString() << u"---\na: 1\nBody"_s;
    QTest::newRow("not first") << u"\n---\na: 1\n---\n"_s << false << QString()
                               << u"\n---\na: 1\n---\n"_s;
    QTest::newRow("dots do not open")
        << u"...\na: 1\n...\n"_s << false << QString() << u"...\na: 1\n...\n"_s;
    QTest::newRow("rule in body") << u"Text\n\n---\n\nMore"_s << false << QString()
                                  << u"Text\n\n---\n\nMore"_s;
}

void TestFrontMatter::splits()
{
    QFETCH(QString, note);
    QFETCH(bool, has);
    QFETCH(QString, yaml);
    QFETCH(QString, body);

    const jot::NoteParts parts = jot::splitFrontMatter(note);
    QCOMPARE(parts.hasFrontMatter, has);
    QCOMPARE(parts.frontMatter.toString(), yaml);
    QCOMPARE(parts.body.toString(), body);
}

QTEST_GUILESS_MAIN(TestFrontMatter)
#include "tst_frontmatter.moc"
