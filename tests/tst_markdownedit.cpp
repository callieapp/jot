#include "jot/MarkdownEdit.h"

#include <QTest>

using namespace jot;
using namespace jot::markdownedit;
using namespace Qt::StringLiterals;

/// Formatting as text: `|` marks the cursor, or `[` and `]` the selection, before and after.
class TestMarkdownEdit : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void wrapsAndUnwraps_data();
    void wrapsAndUnwraps();
    void stylesLines_data();
    void stylesLines();
    void makesLinks_data();
    void makesLinks();
    void fencesCode_data();
    void fencesCode();
    void setsFrontMatterKeepingTheSelection();
    void editsAreAsSmallAsTheyCanBe();
};

namespace {

struct Marked
{
    QString text;
    qsizetype from;
    qsizetype to;
};

Marked unmark(QString text)
{
    if (const qsizetype cursor = text.indexOf(u'|'); cursor >= 0) {
        text.remove(cursor, 1);
        return {text, cursor, cursor};
    }
    const qsizetype from = text.indexOf(u'[');
    text.remove(from, 1);
    const qsizetype to = text.indexOf(u']', from);
    text.remove(to, 1);
    return {text, from, to};
}

QString applied(const QString &text, const Edit &edit)
{
    QString out = text;
    out.replace(edit.start, edit.end - edit.start, edit.text);
    if (edit.selectionStart == edit.selectionEnd)
        return out.insert(edit.selectionStart, u'|');
    return out.insert(edit.selectionEnd, u']').insert(edit.selectionStart, u'[');
}

} // namespace

// Brackets would read as selection marks, so links are tested with their own markers.
void TestMarkdownEdit::wrapsAndUnwraps_data()
{
    QTest::addColumn<QString>("marker");
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("after");
    QTest::newRow("bold a selection")
        << u"**"_s << u"say [hello] there"_s << u"say **[hello]** there"_s;
    QTest::newRow("unbold it") << u"**"_s << u"say **[hello]** there"_s << u"say [hello] there"_s;
    QTest::newRow("unbold with marks selected")
        << u"**"_s << u"say [**hello**] there"_s << u"say [hello] there"_s;
    QTest::newRow("spaces stay outside")
        << u"**"_s << u"say[ hello ]there"_s << u"say **[hello]** there"_s;
    QTest::newRow("the word at the cursor") << u"*"_s << u"say hel|lo"_s << u"say *hel|lo*"_s;
    QTest::newRow("unitalic the word") << u"*"_s << u"say *hel|lo*"_s << u"say hel|lo"_s;
    QTest::newRow("italic inside bold") << u"*"_s << u"**[hi]**"_s << u"***[hi]***"_s;
    QTest::newRow("bold is not italic") << u"*"_s << u"**hi|**"_s << u"***hi|***"_s;
    QTest::newRow("unbold keeps italic") << u"**"_s << u"***[hi]***"_s << u"*[hi]*"_s;
    QTest::newRow("a pair to type in") << u"`"_s << u"run | now"_s << u"run `|` now"_s;
    QTest::newRow("an empty pair goes") << u"`"_s << u"run `|` now"_s << u"run | now"_s;
    QTest::newRow("each line") << u"~~"_s << u"[one\n\ntwo]"_s << u"[~~one~~\n\n~~two~~]"_s;
    QTest::newRow("each line, off") << u"~~"_s << u"[~~one~~\n~~two~~]"_s << u"[one\ntwo]"_s;
}

void TestMarkdownEdit::wrapsAndUnwraps()
{
    QFETCH(QString, marker);
    QFETCH(QString, before);
    QFETCH(QString, after);
    const Marked in = unmark(before);
    QCOMPARE(applied(in.text, wrap(in.text, in.from, in.to, marker)), after);
}

void TestMarkdownEdit::stylesLines_data()
{
    QTest::addColumn<int>("style");
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("after");
    const auto row = [](const char *name, LineStyle style, const QString &before,
                        const QString &after) {
        QTest::newRow(name) << int(style) << before << after;
    };
    row("a task on an empty line", LineStyle::Task, u"one\n|\ntwo"_s, u"one\n- [ ] |\ntwo"_s);
    row("a task keeps the cursor", LineStyle::Task, u"buy mi|lk"_s, u"- [ ] buy mi|lk"_s);
    row("a bullet becomes a task", LineStyle::Task, u"  - buy |milk"_s, u"  - [ ] buy |milk"_s);
    row("a task goes", LineStyle::Task, u"- [x] buy |milk"_s, u"buy |milk"_s);
    row("numbers count", LineStyle::Numbered, u"[a\n\nb\nc]"_s, u"[1. a\n\n2. b\n3. c]"_s);
    row("bullets off", LineStyle::Bullet, u"[- a\n* b]"_s, u"[a\nb]"_s);
    row("mixed lists all get it", LineStyle::Bullet, u"[- a\nb]"_s, u"[- a\n- b]"_s);
    row("the line after a selection is left", LineStyle::Quote, u"[a\n]b"_s, u"[> a\n]b"_s);
    row("quote keeps the list", LineStyle::Quote, u"- a|"_s, u"> - a|"_s);
    row("heading", LineStyle::Heading2, u"Ti|tle"_s, u"## Ti|tle"_s);
    row("heading level changes", LineStyle::Heading1, u"### Ti|tle"_s, u"# Ti|tle"_s);
    row("same heading goes", LineStyle::Heading2, u"## Ti|tle"_s, u"Ti|tle"_s);
    row("cursor in the marks goes to the text", LineStyle::Heading1, u"#|# Title"_s, u"# |Title"_s);
}

void TestMarkdownEdit::stylesLines()
{
    QFETCH(int, style);
    QFETCH(QString, before);
    QFETCH(QString, after);
    const Marked in = unmark(before);
    QCOMPARE(applied(in.text, styleLines(in.text, in.from, in.to, LineStyle(style))), after);
}

void TestMarkdownEdit::makesLinks_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<QString>("after");
    QTest::addColumn<int>("cursor");
    QTest::newRow("a label") << u"see docs now"_s << 4 << 8 << u"see [docs]() now"_s << 11;
    QTest::newRow("a url") << u"at https://x.org"_s << 3 << 16 << u"at [](https://x.org)"_s << 4;
    QTest::newRow("nothing") << u"ab"_s << 1 << 1 << u"a[]()b"_s << 2;
}

void TestMarkdownEdit::makesLinks()
{
    QFETCH(QString, text);
    QFETCH(int, from);
    QFETCH(int, to);
    QFETCH(QString, after);
    QFETCH(int, cursor);
    const Edit edit = link(text, from, to);
    QCOMPARE(QString(text).replace(edit.start, edit.end - edit.start, edit.text), after);
    QCOMPARE(edit.selectionStart, cursor);
    QCOMPARE(edit.selectionEnd, cursor);
}

void TestMarkdownEdit::fencesCode_data()
{
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("after");
    QTest::newRow("fence") << u"a\nx = |1\nb"_s << u"a\n```\nx = |1\n```\nb"_s;
    QTest::newRow("unfence") << u"a\n```\nx = |1\n```\nb"_s << u"a\nx = |1\nb"_s;
    QTest::newRow("unfence a tagged fence") << u"```cpp\n[x\ny]\n```"_s << u"[x\ny]"_s;
    QTest::newRow("unfence from the middle") << u"```\na\n|b\nc\n```"_s << u"a\n|b\nc"_s;
    QTest::newRow("unfence from a fence line") << u"|```\na\n```"_s << u"|a"_s;
    QTest::newRow("between two blocks")
        << u"```\na\n```\n|b\n```\nc\n```"_s << u"```\na\n```\n```\n|b\n```\n```\nc\n```"_s;
    QTest::newRow("under a closing fence")
        << u"```\na\n```\n|b"_s << u"```\na\n```\n```\n|b\n```"_s;
    QTest::newRow("an unclosed block") << u"```\na\n|b"_s << u"a\n|b"_s;
}

void TestMarkdownEdit::fencesCode()
{
    QFETCH(QString, before);
    QFETCH(QString, after);
    const Marked in = unmark(before);
    QCOMPARE(applied(in.text, codeBlock(in.text, in.from, in.to)), after);
}

void TestMarkdownEdit::setsFrontMatterKeepingTheSelection()
{
    const QString note = u"Buy milk.\n"_s;
    const Edit made = setKey(note, 4, 8, u"todo"_s, u"open"_s);
    QCOMPARE(applied(note, made), u"---\ntodo: open\n---\nBuy [milk].\n"_s);

    const QString todo = u"---\ntodo: open\ndue: 2026-10-20\n---\nBuy milk.\n"_s;
    const Edit dropped = setKey(todo, 37, 37, u"due"_s, QVariant());
    QCOMPARE(applied(todo, dropped), u"---\ntodo: open\n---\nBu|y milk.\n"_s);
}

void TestMarkdownEdit::editsAreAsSmallAsTheyCanBe()
{
    const Edit edit = wrap(u"a long line with one word"_s, 7, 11, u"**"_s);
    QCOMPARE(edit.start, 7);
    QCOMPARE(edit.end, 11);
    QCOMPARE(edit.text, u"**line**"_s);
}

QTEST_GUILESS_MAIN(TestMarkdownEdit)
#include "tst_markdownedit.moc"
