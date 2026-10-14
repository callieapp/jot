#include "jot/Settings.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace jot;
using namespace Qt::StringLiterals;

class TestSettings : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void startsWithDefaults();
    void savesAtOnceAndReadsBack();
    void signalsOnlyOnAChange();
    void refusesAnUnknownSort();
};

void TestSettings::startsWithDefaults()
{
    QTemporaryDir dir;
    const Settings settings(dir.filePath(u"settings.ini"_s));
    QCOMPARE(settings.vaultPath(), QString());
    QCOMPARE(settings.theme(), QString());
    QCOMPARE(settings.noteSort(), u"updated"_s);
    QVERIFY(settings.collapsedNotebooks().isEmpty());
    QVERIFY(settings.checkForUpdates());
}

void TestSettings::savesAtOnceAndReadsBack()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(u"settings.ini"_s);
    {
        Settings settings(path);
        settings.setVaultPath(u"/home/me/Notes"_s);
        settings.setTheme(u"paper"_s);
        settings.setNoteSort(u"title"_s);
        settings.setCollapsedNotebooks({u"Ashfall"_s, u"Ashfall/Cast"_s});
        settings.setCheckForUpdates(false);
    }
    const Settings again(path);
    QCOMPARE(again.vaultPath(), u"/home/me/Notes"_s);
    QCOMPARE(again.theme(), u"paper"_s);
    QCOMPARE(again.noteSort(), u"title"_s);
    QCOMPARE(again.collapsedNotebooks(), (QStringList{u"Ashfall"_s, u"Ashfall/Cast"_s}));
    QVERIFY(!again.checkForUpdates());
}

void TestSettings::signalsOnlyOnAChange()
{
    QTemporaryDir dir;
    Settings settings(dir.filePath(u"settings.ini"_s));
    QSignalSpy changed(&settings, &Settings::vaultPathChanged);
    settings.setVaultPath(u"/a"_s);
    settings.setVaultPath(u"/a"_s);
    QCOMPARE(changed.size(), 1);
}

void TestSettings::refusesAnUnknownSort()
{
    QTemporaryDir dir;
    Settings settings(dir.filePath(u"settings.ini"_s));
    settings.setNoteSort(u"sideways"_s);
    QCOMPARE(settings.noteSort(), u"updated"_s);
}

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
