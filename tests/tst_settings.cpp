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
    void resetKeepsTheVault();
    void refusesAnUnknownNoteFont();
    void keepsColumnWidthsInBounds();
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

void TestSettings::resetKeepsTheVault()
{
    QTemporaryDir dir;
    Settings settings(dir.filePath(u"settings.ini"_s));
    settings.setVaultPath(u"/home/me/Notes"_s);
    settings.setTheme(u"paper"_s);
    settings.setNoteSort(u"title"_s);
    settings.setCheckForUpdates(false);
    settings.setSourceMode(true);
    settings.setNoteFont(u"serif"_s);
    settings.setCollapsedNotebooks({u"Work"_s});
    QSignalSpy themeChanged(&settings, &Settings::themeChanged);

    settings.reset();

    QCOMPARE(themeChanged.count(), 1);
    QCOMPARE(settings.theme(), QString());
    QCOMPARE(settings.noteSort(), u"updated"_s);
    QVERIFY(settings.checkForUpdates());
    QVERIFY(!settings.sourceMode());
    QCOMPARE(settings.noteFont(), QString());
    QCOMPARE(settings.vaultPath(), u"/home/me/Notes"_s);
    QCOMPARE(settings.collapsedNotebooks(), QStringList{u"Work"_s});
    QCOMPARE(Settings(dir.filePath(u"settings.ini"_s)).noteSort(), u"updated"_s);
}

void TestSettings::refusesAnUnknownNoteFont()
{
    QTemporaryDir dir;
    Settings settings(dir.filePath(u"settings.ini"_s));
    settings.setNoteFont(u"mono"_s);
    settings.setNoteFont(u"comic"_s);
    QCOMPARE(settings.noteFont(), u"mono"_s);
    QCOMPARE(Settings(dir.filePath(u"settings.ini"_s)).noteFont(), u"mono"_s);
}

void TestSettings::keepsColumnWidthsInBounds()
{
    QTemporaryDir dir;
    Settings settings(dir.filePath(u"settings.ini"_s));
    QCOMPARE(settings.sidebarWidth(), 0);
    settings.setSidebarWidth(300);
    QCOMPARE(settings.sidebarWidth(), 300);
    settings.setSidebarWidth(20);
    QCOMPARE(settings.sidebarWidth(), 180);
    settings.setNoteListWidth(5000);
    QCOMPARE(settings.noteListWidth(), 640);
    QCOMPARE(Settings(dir.filePath(u"settings.ini"_s)).noteListWidth(), 640);
    // 0 is the theme's own width, which reset goes back to.
    settings.reset();
    QCOMPARE(settings.sidebarWidth(), 0);
    QCOMPARE(settings.noteListWidth(), 0);
}

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
