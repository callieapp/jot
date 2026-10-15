#include "jot/FrontMatter.h"
#include "jot/NoteScan.h"

#include <QTest>

using namespace jot;
using namespace Qt::StringLiterals;

class TestNoteScan : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void findsEveryKindOfLink();
    void leavesUrlsAndAnchorsOut();
    void findsNestedTagsButNotHeadingsOrNumbers();
    void skipsCode();
    void findsTasks();
    void countsLinesFromTheTopOfTheFile();
    void ticksATaskAndNothingElse();
};

void TestNoteScan::findsEveryKindOfLink()
{
    const NoteScan scan = scanNote(uR"(See [[Mara]], [[Cast/Soren|her brother]] and [[Mara#Voice]].
![[map.png]] and [the ferry](Places/The%20Ferry.md "title") and ![sketch](Mara.assets/sketch.png)
[odd](<With spaces.md>) and [[ spaced ]]
)"_s);
    const QList<NoteScan::Link> expected{
        {u"Mara"_s, u"wiki"_s, 1},
        {u"Cast/Soren"_s, u"wiki"_s, 1},
        {u"Mara#Voice"_s, u"wiki"_s, 1},
        {u"map.png"_s, u"embed"_s, 2},
        {u"Places/The%20Ferry.md"_s, u"link"_s, 2},
        {u"Mara.assets/sketch.png"_s, u"embed"_s, 2},
        {u"With spaces.md"_s, u"link"_s, 3},
        {u"spaced"_s, u"wiki"_s, 3},
    };
    QCOMPARE(scan.links, expected);
}

void TestNoteScan::leavesUrlsAndAnchorsOut()
{
    const NoteScan scan = scanNote(
        u"[site](https://callieapp.org) [mail](mailto:a@b.c) [up](#top) <https://x.y>\n"_s);
    QVERIFY(scan.links.isEmpty());
}

void TestNoteScan::findsNestedTagsButNotHeadingsOrNumbers()
{
    const NoteScan scan = scanNote(uR"(# Heading
## Another
Tagged #ashfall and #cast/lead, (#idea), #2026 is a number, #v2 is fine.
Not a tag: a#b, [[#Heading]], issue #12, [link](#anchor)
#Ashfall again
)"_s);
    QCOMPARE(scan.tags, (QStringList{u"ashfall"_s, u"cast/lead"_s, u"idea"_s, u"v2"_s}));
}

void TestNoteScan::skipsCode()
{
    const NoteScan scan = scanNote(uR"(```cpp
#include <x> [[NotALink]]
```
~~~
- [ ] not a task
~~~
Inline `#notatag [[NotALink]]` but #real and [[Real]].
    indented #code [[NotALink]]
)"_s);
    QCOMPARE(scan.tags, QStringList{u"real"_s});
    QCOMPARE(scan.links, (QList<NoteScan::Link>{{u"Real"_s, u"wiki"_s, 7}}));
    QVERIFY(scan.tasks.isEmpty());
}

void TestNoteScan::findsTasks()
{
    const NoteScan scan = scanNote(uR"(- [ ] buy paper
- [x] find shop
  * [X] nested and done
1. [ ] numbered
- [] not a task
- plain
)"_s);
    const QList<NoteScan::Task> expected{{1, false, u"buy paper"_s},
                                         {2, true, u"find shop"_s},
                                         {3, true, u"nested and done"_s},
                                         {4, false, u"numbered"_s}};
    QCOMPARE(scan.tasks, expected);
}

void TestNoteScan::countsLinesFromTheTopOfTheFile()
{
    const QString note = u"---\ntitle: x\n---\nline four [[A]]\r\n- [ ] five\r\n"_s;
    const NoteParts parts = splitFrontMatter(note);
    const int first = bodyFirstLine(note, parts.body);
    QCOMPARE(first, 4);
    const NoteScan scan = scanNote(parts.body, first);
    QCOMPARE(scan.links.first().line, 4);
    QCOMPARE(scan.tasks.first().line, 5);
    QCOMPARE(scan.tasks.first().text, u"five"_s);
}

void TestNoteScan::ticksATaskAndNothingElse()
{
    const QString note = u"---\na: 1\n---\n- [ ] one\r\n  * [X] two\nplain\n3. [ ] three"_s;
    QCOMPARE(setTaskDone(note, 4, true),
             std::optional(u"---\na: 1\n---\n- [x] one\r\n  * [X] two\nplain\n3. [ ] three"_s));
    QCOMPARE(setTaskDone(note, 5, false),
             std::optional(u"---\na: 1\n---\n- [ ] one\r\n  * [ ] two\nplain\n3. [ ] three"_s));
    QCOMPARE(setTaskDone(note, 7, true),
             std::optional(u"---\na: 1\n---\n- [ ] one\r\n  * [X] two\nplain\n3. [x] three"_s));
    QVERIFY(!setTaskDone(note, 6, true));
    QVERIFY(!setTaskDone(note, 99, true));
}

QTEST_GUILESS_MAIN(TestNoteScan)
#include "tst_notescan.moc"
