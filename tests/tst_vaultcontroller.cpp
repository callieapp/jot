#include "VaultController.h"

#include "jot/Settings.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace jot;
using namespace Qt::StringLiterals;

class TestVaultController : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void startsClosed();
    void opensAFolderAndRemembersIt();
    void refusesWhatIsNotAFolder();
    void listsNotebooksThatFold();
    void listsNotesByNotebookTagAndSearch();
    void ordersNotesAsAsked();
    void switcherFindsByTitleAliasAndPath();
    void showsTheCurrentNote();
    void followsChangesOnDisk();
    void savesShortlyAfterTypingStops();
    void savesBeforeSwitchingNotes();
    void keepsTheOtherVersionWhenBothChanged();
    void makesNewNotesInTheNotebookShown();
    void renamingRewritesLinksAndKeepsTheTitle();
    void trashesTheNoteShown();
    void followsLinksAndMakesMissingNotes();
    void keepsLineEndingsAndTheByteOrderMark();
    void aRenameThatFailsHalfwayShowsWhereTheNoteIs();
    void aFailedSaveIsShownAndRetried();

private:
    QStringList column(const QAbstractItemModel &model, const QByteArray &role) const;

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<VaultController> m_vault;
    QString m_root;
};

void TestVaultController::init()
{
    m_dir = std::make_unique<QTemporaryDir>();
    m_root = m_dir->filePath(u"Notes"_s);
    const QString sample = QStringLiteral(JOT_SAMPLE_VAULT);
    QDirIterator files(sample, QDir::Files, QDirIterator::Subdirectories);
    while (files.hasNext()) {
        const QString from = files.next();
        const QString to = m_root + from.mid(sample.size());
        QVERIFY(QDir().mkpath(QFileInfo(to).path()));
        QVERIFY(QFile::copy(from, to));
    }
    m_settings = std::make_unique<Settings>(m_dir->filePath(u"settings.ini"_s));
    m_vault = std::make_unique<VaultController>(*m_settings);
    m_vault->setIndexPath(m_dir->filePath(u"index.sqlite"_s));
}

void TestVaultController::cleanup()
{
    m_vault.reset();
}

QStringList TestVaultController::column(const QAbstractItemModel &model,
                                        const QByteArray &role) const
{
    const int id = model.roleNames().key(role);
    QStringList values;
    for (int row = 0; row < model.rowCount(); ++row)
        values.append(model.data(model.index(row, 0), id).toString());
    return values;
}

void TestVaultController::startsClosed()
{
    QVERIFY(!m_vault->isOpen());
    QVERIFY(!m_vault->openLast());
    QCOMPARE(m_vault->notebooks()->rowCount(), 0);
    QCOMPARE(m_vault->notes()->rowCount(), 0);
}

void TestVaultController::opensAFolderAndRemembersIt()
{
    QSignalSpy opened(m_vault.get(), &VaultController::openChanged);
    QVERIFY(m_vault->openFolder(QUrl::fromLocalFile(m_root)));
    QCOMPARE(opened.size(), 1);
    QVERIFY(m_vault->isOpen());
    QCOMPARE(m_vault->name(), u"Notes"_s);
    QCOMPARE(m_vault->noteCount(), 8);
    QCOMPARE(m_settings->vaultPath(), m_root);

    VaultController again(*m_settings);
    again.setIndexPath(m_dir->filePath(u"index.sqlite"_s));
    m_vault.reset();
    QVERIFY(again.openLast());
    QCOMPARE(again.noteCount(), 8);
}

void TestVaultController::refusesWhatIsNotAFolder()
{
    QFile file(m_dir->filePath(u"file"_s));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"could not open the vault.*"_s));
    QVERIFY(!m_vault->openPath(file.fileName()));
    QVERIFY(!m_vault->isOpen());
    QVERIFY(!m_vault->error().isEmpty());
}

void TestVaultController::listsNotebooksThatFold()
{
    QVERIFY(m_vault->openPath(m_root));
    NotebookModel &notebooks = *m_vault->notebooks();
    QCOMPARE(column(notebooks, "path"),
             (QStringList{QString(), u"Ashfall"_s, u"Ashfall/Characters"_s, u"Ashfall/Places"_s,
                          u"Inbox"_s, u"Recipes"_s}));
    QCOMPARE(column(notebooks, "count"),
             (QStringList{u"8"_s, u"4"_s, u"2"_s, u"1"_s, u"1"_s, u"2"_s}));
    QCOMPARE(column(notebooks, "depth").at(2), u"1"_s);

    notebooks.toggle(u"Ashfall"_s);
    QCOMPARE(column(notebooks, "path"),
             (QStringList{QString(), u"Ashfall"_s, u"Inbox"_s, u"Recipes"_s}));
    QCOMPARE(m_settings->collapsedNotebooks(), QStringList{u"Ashfall"_s});
    notebooks.toggle(u"Ashfall"_s);
    QCOMPARE(notebooks.rowCount(), 6);
}

void TestVaultController::listsNotesByNotebookTagAndSearch()
{
    QVERIFY(m_vault->openPath(m_root));
    NoteListModel &notes = *m_vault->notes();
    QCOMPARE(notes.rowCount(), 8);

    notes.setNotebook(u"Ashfall/Characters"_s);
    QCOMPARE(notes.rowCount(), 2);
    notes.setTag(u"cast"_s);
    QCOMPARE(notes.notebook(), QString());
    QCOMPARE(notes.rowCount(), 2);
    notes.setTag(u"food"_s);
    QCOMPARE(notes.rowCount(), 2);

    // A search stays inside the notebook or tag shown.
    notes.setQuery(u"starter"_s);
    QCOMPARE(column(notes, "path"), QStringList{u"Recipes/Sourdough.md"_s});
    notes.setNotebook(u"Ashfall"_s);
    QCOMPARE(notes.rowCount(), 0);
    // A quote left open neither swallows the notebook nor the words.
    notes.setNotebook(u"Recipes"_s);
    notes.setQuery(u"\"starter"_s);
    QCOMPARE(column(notes, "path"), QStringList{u"Recipes/Sourdough.md"_s});
    notes.setNotebook(u"Ashfall"_s);
    notes.setQuery(u"waxed ledger"_s);
    QCOMPARE(column(notes, "path"), QStringList{u"Ashfall/Characters/Mara Vey.md"_s});
    QVERIFY(column(notes, "snippet").first().contains(u"<mark>"_s));
}

void TestVaultController::ordersNotesAsAsked()
{
    QVERIFY(m_vault->openPath(m_root));
    NoteListModel &notes = *m_vault->notes();
    notes.setNotebook(u"Recipes"_s);
    notes.setOrder(u"title"_s);
    QCOMPARE(column(notes, "title"), (QStringList{u"Pancakes"_s, u"Sourdough"_s}));
    QCOMPARE(m_settings->noteSort(), u"title"_s);

    // Recently changed first.
    QTest::qWait(20);
    QFile touched(m_root + u"/Recipes/Pancakes.md"_s);
    QVERIFY(touched.open(QIODevice::Append));
    touched.write("more\n");
    touched.close();
    notes.setOrder(u"updated"_s);
    QSignalSpy changed(m_vault.get(), &VaultController::notesChanged);
    QVERIFY(changed.wait(3000));
    QCOMPARE(column(notes, "title").first(), u"Pancakes"_s);
}

void TestVaultController::switcherFindsByTitleAliasAndPath()
{
    QVERIFY(m_vault->openPath(m_root));
    SwitcherModel &switcher = *m_vault->switcher();
    QCOMPARE(switcher.rowCount(), 8);

    switcher.setQuery(u"sour"_s);
    QCOMPARE(column(switcher, "title").first(), u"Sourdough"_s);
    switcher.setQuery(u"ferrywoman"_s);
    QCOMPARE(column(switcher, "title").first(), u"Mara Vey"_s);
    QCOMPARE(column(switcher, "alias").first(), u"the Ferrywoman"_s);
    switcher.setQuery(u"places"_s);
    QCOMPARE(column(switcher, "path").first(), u"Ashfall/Places/The Ferry.md"_s);
    switcher.setQuery(u"zzz"_s);
    QCOMPARE(switcher.rowCount(), 0);
}

void TestVaultController::showsTheCurrentNote()
{
    QVERIFY(m_vault->openPath(m_root));
    QSignalSpy changed(m_vault.get(), &VaultController::currentChanged);
    m_vault->setCurrent(u"Ashfall/Characters/Mara Vey.md"_s);
    QCOMPARE(changed.size(), 1);
    QCOMPARE(m_vault->currentTitle(), u"Mara Vey"_s);
    QVERIFY(m_vault->currentText().contains(u"waxed ledger"_s));
}

void TestVaultController::followsChangesOnDisk()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Welcome.md"_s);
    QSignalSpy changed(m_vault.get(), &VaultController::notesChanged);

    QFile created(m_root + u"/Inbox/Idea.md"_s);
    QVERIFY(created.open(QIODevice::WriteOnly));
    created.write("#idea a new thought\n");
    created.close();
    QFile welcome(m_root + u"/Welcome.md"_s);
    QVERIFY(welcome.open(QIODevice::WriteOnly | QIODevice::Truncate));
    welcome.write("Rewritten elsewhere.\n");
    welcome.close();

    QTRY_COMPARE_WITH_TIMEOUT(m_vault->noteCount(), 9, 3000);
    QVERIFY(m_vault->tags().contains(u"idea"_s));
    QTRY_VERIFY_WITH_TIMEOUT(m_vault->currentText().contains(u"Rewritten"_s), 3000);
    QVERIFY(!changed.isEmpty());
}

namespace {

QString readFile(const QString &path)
{
    QString error;
    return notefile::read(path, &error).value_or(error);
}

} // namespace

void TestVaultController::savesShortlyAfterTypingStops()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setSaveDelay(50);
    m_vault->setCurrent(u"Welcome.md"_s);
    m_vault->edit(u"First draft\n"_s);
    QVERIFY(m_vault->unsaved());
    QVERIFY(readFile(m_root + u"/Welcome.md"_s) != u"First draft\n"_s);
    QTRY_VERIFY_WITH_TIMEOUT(!m_vault->unsaved(), 2000);
    QCOMPARE(readFile(m_root + u"/Welcome.md"_s), u"First draft\n"_s);

    // Jot's own save is not mistaken for someone else's.
    QSignalSpy current(m_vault.get(), &VaultController::currentChanged);
    QTest::qWait(700);
    QCOMPARE(current.size(), 0);
}

void TestVaultController::savesBeforeSwitchingNotes()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setSaveDelay(60000);
    m_vault->setCurrent(u"Welcome.md"_s);
    m_vault->edit(u"Kept\n"_s);
    m_vault->setCurrent(u"Recipes/Pancakes.md"_s);
    QCOMPARE(readFile(m_root + u"/Welcome.md"_s), u"Kept\n"_s);
    QVERIFY(!m_vault->unsaved());
}

void TestVaultController::keepsTheOtherVersionWhenBothChanged()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setSaveDelay(60000);
    m_vault->setCurrent(u"Welcome.md"_s);
    m_vault->edit(u"Mine\n"_s);
    QString error;
    QVERIFY(notefile::write(m_root + u"/Welcome.md"_s, u"Theirs\n"_s, &error));

    QTRY_COMPARE_WITH_TIMEOUT(QDir(m_root).entryList({u"Welcome (conflict *).md"_s}).size(), 1,
                              3000);
    const QString copy = QDir(m_root).entryList({u"Welcome (conflict *).md"_s}).first();
    QCOMPARE(readFile(m_root + u'/' + copy), u"Theirs\n"_s);
    m_vault->saveNow();
    QCOMPARE(readFile(m_root + u"/Welcome.md"_s), u"Mine\n"_s);
}

void TestVaultController::makesNewNotesInTheNotebookShown()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->notes()->setNotebook(u"Recipes"_s);
    QCOMPARE(m_vault->newNote(), u"Recipes/Untitled.md"_s);
    QCOMPARE(m_vault->newNote(), u"Recipes/Untitled 2.md"_s);
    QCOMPARE(m_vault->current(), u"Recipes/Untitled 2.md"_s);
    QCOMPARE(m_vault->notes()->rowCount(), 4);
    QVERIFY(m_vault->currentText().startsWith(u"---\ncreated: "_s));
}

void TestVaultController::renamingRewritesLinksAndKeepsTheTitle()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Ashfall/Places/The Ferry.md"_s);
    QVERIFY(m_vault->renameCurrent(u"The Rope Ferry"_s));
    QCOMPARE(m_vault->current(), u"Ashfall/Places/The Rope Ferry.md"_s);
    QCOMPARE(m_vault->currentTitle(), u"The Rope Ferry"_s);
    QVERIFY(readFile(m_root + u"/Ashfall/Pitch.md"_s).contains(u"[[The Rope Ferry]]"_s));
    QVERIFY(
        readFile(m_root + u"/Ashfall/Characters/Mara Vey.md"_s).contains(u"[[The Rope Ferry]]"_s));

    // A title the file name cannot hold goes into the front matter, and out again.
    QVERIFY(m_vault->renameCurrent(u"Ferry: the rope one"_s));
    QCOMPARE(m_vault->current(), u"Ashfall/Places/Ferry the rope one.md"_s);
    QCOMPARE(m_vault->currentTitle(), u"Ferry: the rope one"_s);
    QVERIFY(m_vault->currentText().contains(u"title: \"Ferry: the rope one\""_s));
    QVERIFY(m_vault->renameCurrent(u"Ferry"_s));
    QVERIFY(!m_vault->currentText().contains(u"title:"_s));
    QVERIFY(!m_vault->renameCurrent(u"  "_s));
}

void TestVaultController::trashesTheNoteShown()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Recipes/Pancakes.md"_s);
    QVERIFY(m_vault->trashCurrent());
    QVERIFY(!QFileInfo::exists(m_root + u"/Recipes/Pancakes.md"_s));
    QCOMPARE(m_vault->current(), QString());
    QCOMPARE(m_vault->noteCount(), 7);
}

void TestVaultController::followsLinksAndMakesMissingNotes()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Ashfall/Pitch.md"_s);
    QVERIFY(m_vault->follow(u"Mara"_s, true));
    QCOMPARE(m_vault->current(), u"Ashfall/Characters/Mara Vey.md"_s);
    QVERIFY(m_vault->follow(u"Soren"_s, true));
    QCOMPARE(m_vault->current(), u"Ashfall/Characters/Soren.md"_s);

    QVERIFY(m_vault->follow(u"Harbor Master#Debts"_s, true));
    QCOMPARE(m_vault->current(), u"Ashfall/Characters/Harbor Master.md"_s);
    QCOMPARE(m_vault->noteCount(), 9);
    // A Markdown link to nothing makes nothing.
    QVERIFY(!m_vault->follow(u"Nowhere.md"_s, false));
}

void TestVaultController::keepsLineEndingsAndTheByteOrderMark()
{
    QFile file(m_root + u"/Windows.md"_s);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("\xef\xbb\xbfone\r\ntwo\r\n");
    file.close();
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Windows.md"_s);
    // As the editor hands text back: \n lines and no byte order mark.
    m_vault->edit(u"one\ntwo, three\n"_s);
    m_vault->saveNow();
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("\xef\xbb\xbfone\r\ntwo, three\r\n"));
}

void TestVaultController::aRenameThatFailsHalfwayShowsWhereTheNoteIs()
{
    QVERIFY(m_vault->openPath(m_root));
    // The note moves, but its attachments cannot: a folder of that name is in the way.
    QVERIFY(QDir().mkpath(m_root + u"/Recipes/Pancakes.assets"_s));
    QFile photo(m_root + u"/Recipes/Pancakes.assets/photo.png"_s);
    QVERIFY(photo.open(QIODevice::WriteOnly));
    photo.close();
    QVERIFY(QDir().mkpath(m_root + u"/Recipes/Crepes.assets/in-the-way"_s));
    m_vault->setCurrent(u"Recipes/Pancakes.md"_s);
    m_vault->edit(m_vault->currentText() + u"more butter\n"_s);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*could not rename.*"_s));
    QVERIFY(!m_vault->renameCurrent(u"Crepes"_s));
    QVERIFY(!m_vault->problem().isEmpty());
    QCOMPARE(m_vault->current(), u"Recipes/Crepes.md"_s);
    QVERIFY(m_vault->currentText().contains(u"more butter"_s));
    m_vault->edit(m_vault->currentText() + u"and syrup\n"_s);
    m_vault->saveNow();
    // Nothing writes the old path back into being.
    QVERIFY(!QFileInfo::exists(m_root + u"/Recipes/Pancakes.md"_s));
    QVERIFY(readFile(m_root + u"/Recipes/Crepes.md"_s).contains(u"and syrup"_s));
}

void TestVaultController::aFailedSaveIsShownAndRetried()
{
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Inbox/Groceries.md"_s);
    const QString folder = m_root + u"/Inbox"_s;
    // The folder taken away, as a drive that drops out: no user, root included, can save.
    QVERIFY(QDir().rename(folder, m_root + u"/.away"_s));
    m_vault->edit(m_vault->currentText() + u"- [ ] eggs\n"_s);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u".*could not save.*"_s));
    m_vault->saveNow();
    QVERIFY(m_vault->unsaved());
    QVERIFY(m_vault->problem().startsWith(u"Not saved"_s));

    QVERIFY(QDir().rename(m_root + u"/.away"_s, folder));
    QTRY_VERIFY_WITH_TIMEOUT(!m_vault->unsaved(), 8000);
    QVERIFY(m_vault->problem().isEmpty());
    QVERIFY(readFile(folder + u"/Groceries.md"_s).contains(u"eggs"_s));
}

QTEST_GUILESS_MAIN(TestVaultController)
#include "tst_vaultcontroller.moc"
