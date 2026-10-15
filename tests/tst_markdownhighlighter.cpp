#include "MarkdownHighlighter.h"

#include <QImage>
#include <QTemporaryDir>
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>

using namespace jot;
using namespace Qt::StringLiterals;

class TestMarkdownHighlighter : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();

    void headingsGrowAndFoldTheirMarks();
    void marksShowOnTheCursorsLine();
    void emphasisAndCode();
    void linksShowTheirTextOrLabel();
    void fencedCodeCarriesAcrossLines();
    void frontMatterIsNotMarkdown();
    void doneTasksAreStruckThrough();
    void tablesAreMonospaceAndKeepTheirMarks();
    void sourceModeShowsTheMarkdownAsWritten();
    void saysWhatIsUnderAPosition();
    void neverChangesTheText();
    void reservesRoomForImagesOnTheirOwnLines();

private:
    /// Sets the text and styles it at once, as the event loop would in the app.
    void setText(const QString &text);
    QTextCharFormat formatAt(int position) const;
    int find(const QString &needle, int from = 0) const;

    std::unique_ptr<QTextDocument> m_document;
    MarkdownHighlighter *m_highlighter = nullptr;
    MarkdownHighlighter::Look m_look;
};

void TestMarkdownHighlighter::init()
{
    m_document = std::make_unique<QTextDocument>();
    m_highlighter = new MarkdownHighlighter(m_document.get());
    m_look.text = QColor(u"#eeeeee"_s);
    m_look.muted = QColor(u"#aaaaaa"_s);
    m_look.faint = QColor(u"#777777"_s);
    m_look.accent = QColor(u"#f08fb8"_s);
    m_look.codeBackground = QColor(u"#333333"_s);
    m_look.monoFamily = u"Mono Sample"_s;
    m_look.displayFamily = u"Display Sample"_s;
    m_look.pixelSize = 16;
    m_highlighter->setLook(m_look);
    // No cursor anywhere yet, as when the document first loads.
    m_highlighter->setCursorPosition(-1);
}

void TestMarkdownHighlighter::setText(const QString &text)
{
    m_document->setPlainText(text);
    m_highlighter->rehighlight();
}

QTextCharFormat TestMarkdownHighlighter::formatAt(int position) const
{
    const QTextBlock block = m_document->findBlock(position);
    const int at = position - block.position();
    for (const QTextLayout::FormatRange &range : block.layout()->formats()) {
        if (at >= range.start && at < range.start + range.length)
            return range.format;
    }
    return {};
}

int TestMarkdownHighlighter::find(const QString &needle, int from) const
{
    return int(m_document->toPlainText().indexOf(needle, from));
}

void TestMarkdownHighlighter::headingsGrowAndFoldTheirMarks()
{
    setText(u"Intro\n# Title\n### Small\n"_s);
    const QTextCharFormat title = formatAt(find(u"Title"_s));
    QVERIFY(title.property(QTextFormat::FontPixelSize).toInt() > 24);
    QCOMPARE(title.fontFamilies().toStringList(), QStringList{u"Display Sample"_s});
    QVERIFY(formatAt(find(u"Small"_s)).property(QTextFormat::FontPixelSize).toInt() > 16);
    // The # is there but folded away off the cursor's line.
    QCOMPARE(formatAt(find(u"# "_s)).foreground().color(), QColor(Qt::transparent));
    QCOMPARE(formatAt(find(u"# "_s)).property(QTextFormat::FontPixelSize).toInt(), 1);
    QVERIFY(!formatAt(find(u"Intro"_s)).hasProperty(QTextFormat::FontPixelSize));
}

void TestMarkdownHighlighter::marksShowOnTheCursorsLine()
{
    setText(u"one **two**\nthree **four**\n"_s);
    m_highlighter->setCursorPosition(find(u"three"_s));
    QCOMPARE(formatAt(find(u"**four"_s)).foreground().color(), m_look.faint);
    QCOMPARE(formatAt(find(u"**two"_s)).foreground().color(), QColor(Qt::transparent));

    m_highlighter->setCursorPosition(0);
    QCOMPARE(formatAt(find(u"**two"_s)).foreground().color(), m_look.faint);
    QCOMPARE(formatAt(find(u"**four"_s)).foreground().color(), QColor(Qt::transparent));
}

void TestMarkdownHighlighter::emphasisAndCode()
{
    setText(u"**bold** *it* _also_ ~~gone~~ `code **not bold**` #tag a*b*c\n"_s);
    QCOMPARE(formatAt(find(u"bold"_s)).fontWeight(), m_look.boldWeight);
    QVERIFY(formatAt(find(u"it*"_s)).fontItalic());
    QVERIFY(formatAt(find(u"also"_s)).fontItalic());
    QVERIFY(formatAt(find(u"gone"_s)).fontStrikeOut());
    const QTextCharFormat code = formatAt(find(u"code"_s));
    QCOMPARE(code.fontFamilies().toStringList(), QStringList{u"Mono Sample"_s});
    QCOMPARE(code.background().color(), m_look.codeBackground);
    QVERIFY(formatAt(find(u"not bold"_s)).fontWeight() != m_look.boldWeight);
    QCOMPARE(formatAt(find(u"#tag"_s)).foreground().color(), m_look.accent);
    // Stars inside a word are not emphasis here.
    QVERIFY(!formatAt(find(u"b*c"_s)).fontItalic());
}

void TestMarkdownHighlighter::linksShowTheirTextOrLabel()
{
    setText(u"[[Mara]] [[Cast/Soren|Soren]] [the ferry](Places/Ferry.md) "
            u"https://callieapp.org.\n"_s);
    QCOMPARE(formatAt(find(u"Mara"_s)).foreground().color(), m_look.accent);
    QCOMPARE(formatAt(find(u"[["_s)).foreground().color(), QColor(Qt::transparent));
    QCOMPARE(formatAt(find(u"Cast/"_s)).foreground().color(), QColor(Qt::transparent));
    QCOMPARE(formatAt(find(u"Soren]]"_s)).foreground().color(), m_look.accent);
    QVERIFY(formatAt(find(u"the ferry"_s)).fontUnderline());
    QCOMPARE(formatAt(find(u"Places"_s)).foreground().color(), QColor(Qt::transparent));
    QVERIFY(formatAt(find(u"callieapp"_s)).fontUnderline());
    // The full stop after a link is not part of it.
    QVERIFY(!formatAt(find(u".\n"_s, find(u"callieapp.org"_s) + 13)).fontUnderline());
}

void TestMarkdownHighlighter::fencedCodeCarriesAcrossLines()
{
    setText(u"```cpp\n# not a heading\n**not bold**\n```\n# heading\n"_s);
    const QTextCharFormat inside = formatAt(find(u"not a heading"_s));
    QCOMPARE(inside.fontFamilies().toStringList(), QStringList{u"Mono Sample"_s});
    QVERIFY(!inside.hasProperty(QTextFormat::FontPixelSize));
    QVERIFY(formatAt(find(u"not bold"_s)).fontWeight() != m_look.boldWeight);
    QVERIFY(formatAt(find(u"heading\n"_s, find(u"```\n#"_s)))
                .property(QTextFormat::FontPixelSize)
                .toInt() > 16);
    QCOMPARE(m_document->findBlockByNumber(1).userState(), int(MarkdownHighlighter::InFence));
    QCOMPARE(m_document->findBlockByNumber(4).userState(), int(MarkdownHighlighter::Normal));
}

void TestMarkdownHighlighter::frontMatterIsNotMarkdown()
{
    setText(u"---\n# comment: x\ntags: [a]\n---\n# Title\n"_s);
    const QTextCharFormat yaml = formatAt(find(u"comment"_s));
    QCOMPARE(yaml.foreground().color(), m_look.faint);
    QVERIFY(yaml.property(QTextFormat::FontPixelSize).toInt() < 16);
    QVERIFY(formatAt(find(u"Title"_s)).property(QTextFormat::FontPixelSize).toInt() > 24);
    // A --- later on is a rule, not front matter.
    setText(u"Text\n---\n# Title\n"_s);
    QVERIFY(formatAt(find(u"Title"_s)).property(QTextFormat::FontPixelSize).toInt() > 24);
}

void TestMarkdownHighlighter::doneTasksAreStruckThrough()
{
    setText(u"- [ ] open task\n- [x] done task\n"_s);
    QVERIFY(!formatAt(find(u"open task"_s)).fontStrikeOut());
    QVERIFY(formatAt(find(u"done task"_s)).fontStrikeOut());
    QCOMPARE(formatAt(find(u"[x]"_s)).foreground().color(), m_look.accent);
}

void TestMarkdownHighlighter::tablesAreMonospaceAndKeepTheirMarks()
{
    setText(u"| Name     | Kind |\n| -------- | ---- |\n| **Mara** | hero |\nAfter\n"_s);
    const QStringList mono{u"Mono Sample"_s};
    QCOMPARE(formatAt(find(u"Name"_s)).fontFamilies().toStringList(), mono);
    QCOMPARE(formatAt(find(u"hero"_s)).fontFamilies().toStringList(), mono);
    QCOMPARE(formatAt(find(u"| Name"_s)).foreground().color(), m_look.faint);
    QCOMPARE(formatAt(find(u"----"_s)).foreground().color(), m_look.faint);
    // Folding the marks would pull the pipes out of line, so they stay, faded.
    QCOMPARE(formatAt(find(u"**Mara"_s)).foreground().color(), m_look.faint);
    QCOMPARE(formatAt(find(u"Mara"_s)).fontWeight(), m_look.boldWeight);
    QVERIFY(formatAt(find(u"After"_s)).fontFamilies().toStringList() != mono);
}

void TestMarkdownHighlighter::sourceModeShowsTheMarkdownAsWritten()
{
    setText(u"# Title\nSome **bold** text.\n"_s);
    QCOMPARE(formatAt(find(u"**bold"_s)).foreground().color(), QColor(Qt::transparent));

    m_highlighter->setSourceMode(true);
    const QTextCharFormat mark = formatAt(find(u"**bold"_s));
    QCOMPARE(mark.fontFamilies().toStringList(), QStringList{u"Mono Sample"_s});
    QVERIFY(!mark.hasProperty(QTextFormat::FontPixelSize));
    QVERIFY(!mark.hasProperty(QTextFormat::ForegroundBrush));
    QVERIFY(!formatAt(find(u"Title"_s)).hasProperty(QTextFormat::FontPixelSize));
    QVERIFY(formatAt(find(u"bold"_s)).fontWeight() != m_look.boldWeight);

    m_highlighter->setSourceMode(false);
    QCOMPARE(formatAt(find(u"**bold"_s)).foreground().color(), QColor(Qt::transparent));

    // Code and front matter stay out of reach of clicks, as written as when styled.
    setText(u"---\nurl: https://a.example\n---\n```\n- [ ] not a task\n```\n- [ ] task\n"_s);
    m_highlighter->setSourceMode(true);
    QCOMPARE(m_highlighter->hitAt(find(u"[ ] not"_s) + 1).kind, MarkdownHighlighter::Hit::None);
    QCOMPARE(m_highlighter->hitAt(find(u"https"_s) + 2).kind, MarkdownHighlighter::Hit::None);
    QCOMPARE(m_highlighter->hitAt(find(u"[ ] task"_s) + 1).kind, MarkdownHighlighter::Hit::TaskBox);
}

void TestMarkdownHighlighter::saysWhatIsUnderAPosition()
{
    setText(u"- [ ] call [[Mara#Voice|her]] about [the ferry](<Places/The Ferry.md>)\n"
            u"see https://callieapp.org and [site](https://x.y)\n```\n[[NotALink]]\n```\n"_s);
    MarkdownHighlighter::Hit hit = m_highlighter->hitAt(find(u"[ ]"_s) + 1);
    QCOMPARE(hit.kind, MarkdownHighlighter::Hit::TaskBox);
    QCOMPARE(hit.position, find(u"[ ]"_s) + 1);
    hit = m_highlighter->hitAt(find(u"her"_s));
    QCOMPARE(hit.kind, MarkdownHighlighter::Hit::Wikilink);
    QCOMPARE(hit.target, u"Mara#Voice"_s);
    hit = m_highlighter->hitAt(find(u"the ferry"_s));
    QCOMPARE(hit.kind, MarkdownHighlighter::Hit::Link);
    QCOMPARE(hit.target, u"Places/The Ferry.md"_s);
    hit = m_highlighter->hitAt(find(u"callieapp"_s));
    QCOMPARE(hit.kind, MarkdownHighlighter::Hit::Url);
    QCOMPARE(hit.target, u"https://callieapp.org"_s);
    QCOMPARE(m_highlighter->hitAt(find(u"site"_s)).kind, MarkdownHighlighter::Hit::Url);
    QCOMPARE(m_highlighter->hitAt(find(u"call"_s)).kind, MarkdownHighlighter::Hit::None);
    QCOMPARE(m_highlighter->hitAt(find(u"NotALink"_s)).kind, MarkdownHighlighter::Hit::None);
}

void TestMarkdownHighlighter::neverChangesTheText()
{
    const QString text = u"---\na: 1\n---\n# T\n**b** [[l]] `c`\n```\nx\n```\n"_s;
    setText(text);
    m_highlighter->setCursorPosition(5);
    m_highlighter->setCursorPosition(20);
    QCOMPARE(m_document->toPlainText(), text);
    QVERIFY(!m_document->isUndoAvailable());
}

void TestMarkdownHighlighter::reservesRoomForImagesOnTheirOwnLines()
{
    QTemporaryDir dir;
    QImage picture(400, 200, QImage::Format_RGB32);
    picture.fill(Qt::magenta);
    QVERIFY(picture.save(dir.filePath(u"sketch.png"_s)));
    m_look.imageWidth = 300;
    m_highlighter->setLook(m_look);
    m_highlighter->setResolver([&dir](const QString &target, bool) {
        return target == u"sketch.png" ? dir.filePath(u"sketch.png"_s) : QString();
    });
    setText(
        u"text\n![](sketch.png)\n![[sketch.png]]\nsee ![](sketch.png) inline\n![](missing.png)\n"_s);

    const auto image = [this](int block) {
        return dynamic_cast<MarkdownHighlighter::Image *>(
            m_document->findBlockByNumber(block).userData());
    };
    QVERIFY(image(1));
    QCOMPARE(image(1)->size, QSize(300, 150)); // fitted to the width given
    QVERIFY(image(2));                         // wikilink embeds too
    QVERIFY(!image(3));                        // not alone on its line
    QVERIFY(!image(4));                        // nothing to show
    const int bang = find(u"![](sketch"_s);
    QVERIFY(formatAt(bang).property(QTextFormat::FontPixelSize).toInt() > 100);
    QCOMPARE(formatAt(bang + 2).property(QTextFormat::FontPixelSize).toInt(), 1);

    // On the cursor's line the link is there to edit instead.
    m_highlighter->setCursorPosition(bang + 3);
    QVERIFY(!image(1));
    QCOMPARE(formatAt(bang + 4).foreground().color(), m_look.faint);
}

QTEST_MAIN(TestMarkdownHighlighter)
#include "tst_markdownhighlighter.moc"
