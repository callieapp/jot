#include "SettingsForeign.h"
#include "ThemeController.h"
#include "VaultController.h"

#include "jot/Settings.h"

#include <QDirIterator>
#include <QMimeData>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTest>
#include <QtQml/qqmlextensionplugin.h>

#include <functional>

Q_IMPORT_QML_PLUGIN(Jot_UiPlugin)

using namespace jot;
using namespace Qt::StringLiterals;

/// Drives the sidebar and the note list in a window: making and renaming notebooks by hand,
/// and the menus on notes and notebooks.
class TestLists : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { QQuickStyle::setStyle(u"Basic"_s); }
    void init();
    void cleanup();

    void leavesRoomUnderTheTitleBar();
    void renamesANotebookByDoubleClick();
    void escapeKeepsTheOldName();
    void makesANotebookReadyToName();
    void dropsNotesAndNotebooksOnNotebooks();
    void noteMenuMakesATodo();
    void notebookMenuMakesANotebookInside();
    void trashAsksFirst();

private:
    /// The visible menu entry reading `text`, anywhere in the window.
    QQuickItem *menuEntry(const QString &text) const;
    void click(QQuickItem *item, Qt::MouseButton button = Qt::LeftButton);
    /// The sidebar entry labelled `label`, among those shown.
    QQuickItem *entry(const QString &label) const;
    QQuickItem *findItem(const std::function<bool(QQuickItem *)> &match,
                         QQuickItem *from = nullptr) const;
    void type(const QString &text);
    /// Drags `format` holding `data` onto `target`, as the desktop delivers a drag and drop.
    void drop(QQuickItem *target, const QString &format, const QString &data);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<VaultController> m_vault;
    std::unique_ptr<QQmlEngine> m_engine;
    std::unique_ptr<QQuickWindow> m_window;
    std::unique_ptr<QObject> m_sidebar;
    std::unique_ptr<QObject> m_noteList;
    QString m_root;
};

void TestLists::init()
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
    SettingsForeign::s_instance = m_settings.get();
    m_vault = std::make_unique<VaultController>(*m_settings);
    m_vault->setIndexPath(m_dir->filePath(u"index.sqlite"_s));
    m_vault->setSystemTrash(false);
    VaultController::setInstance(m_vault.get());
    QVERIFY(m_vault->openPath(m_root));

    m_engine = std::make_unique<QQmlEngine>();
    QQmlComponent component(m_engine.get());
    component.loadFromModule("Jot.Ui", "Sidebar");
    m_sidebar.reset(component.create());
    QVERIFY2(m_sidebar, qPrintable(component.errorString()));
    QQmlComponent list(m_engine.get());
    list.loadFromModule("Jot.Ui", "NoteList");
    m_noteList.reset(list.create());
    QVERIFY2(m_noteList, qPrintable(list.errorString()));
    m_window = std::make_unique<QQuickWindow>();
    m_window->resize(640, 700);
    auto *item = qobject_cast<QQuickItem *>(m_sidebar.get());
    item->setParentItem(m_window->contentItem());
    item->setSize({320, 700});
    auto *notes = qobject_cast<QQuickItem *>(m_noteList.get());
    notes->setParentItem(m_window->contentItem());
    notes->setPosition({320, 0});
    notes->setSize({320, 700});
    m_window->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_window.get()));
}

void TestLists::cleanup()
{
    m_sidebar.reset();
    m_noteList.reset();
    m_window.reset();
    m_engine.reset();
    VaultController::setInstance(nullptr);
    m_vault.reset();
    SettingsForeign::s_instance = nullptr;
}

QQuickItem *TestLists::findItem(const std::function<bool(QQuickItem *)> &match,
                                QQuickItem *from) const
{
    if (!from)
        from = qobject_cast<QQuickItem *>(m_sidebar.get());
    for (QQuickItem *child : from->childItems()) {
        if (match(child))
            return child;
        if (QQuickItem *found = findItem(match, child))
            return found;
    }
    return nullptr;
}

QQuickItem *TestLists::entry(const QString &label) const
{
    return findItem([&label](QQuickItem *item) {
        return item->isVisible() && item->property("label").toString() == label;
    });
}

QQuickItem *TestLists::menuEntry(const QString &text) const
{
    return findItem(
        [&text](QQuickItem *item) {
            return item->isVisible() && item->inherits("QQuickMenuItem") &&
                   item->property("text").toString() == text;
        },
        m_window->contentItem());
}

void TestLists::click(QQuickItem *item, Qt::MouseButton button)
{
    QTest::mouseClick(m_window.get(), button, {},
                      item->mapToScene({item->width() / 2, item->height() / 2}).toPoint());
}

void TestLists::type(const QString &text)
{
    for (const QChar c : text)
        QTest::keyClick(m_window.get(), c.toLatin1());
}

void TestLists::leavesRoomUnderTheTitleBar()
{
    QQuickItem *todos = entry(u"To-dos"_s);
    QVERIFY(todos);
    QVERIFY(todos->mapToScene({0, 0}).y() >= ThemeController::instance()->space3());
}

void TestLists::renamesANotebookByDoubleClick()
{
    QQuickItem *places = entry(u"Places"_s);
    QVERIFY(places);
    const QPoint centre = places->mapToScene({places->width() / 2, places->height() / 2}).toPoint();
    QTest::mouseDClick(m_window.get(), Qt::LeftButton, {}, centre);
    // The name is selected, so typing replaces it.
    QTRY_VERIFY(m_sidebar->property("renamingPath").toString() == u"Ashfall/Places"_s);
    type(u"Harbor"_s);
    QTest::keyClick(m_window.get(), Qt::Key_Return);

    QTRY_VERIFY(QFileInfo(m_root + u"/Ashfall/Harbor"_s).isDir());
    QVERIFY(!QFileInfo::exists(m_root + u"/Ashfall/Places"_s));
    QCOMPARE(m_sidebar->property("renamingPath").toString(), QString());
    QTRY_VERIFY(entry(u"Harbor"_s));
}

void TestLists::escapeKeepsTheOldName()
{
    QQuickItem *places = entry(u"Places"_s);
    const QPoint centre = places->mapToScene({places->width() / 2, places->height() / 2}).toPoint();
    QTest::mouseDClick(m_window.get(), Qt::LeftButton, {}, centre);
    QTRY_VERIFY(!m_sidebar->property("renamingPath").toString().isEmpty());
    type(u"Nope"_s);
    QTest::keyClick(m_window.get(), Qt::Key_Escape);
    QCOMPARE(m_sidebar->property("renamingPath").toString(), QString());
    QVERIFY(QFileInfo(m_root + u"/Ashfall/Places"_s).isDir());
}

void TestLists::makesANotebookReadyToName()
{
    QQuickItem *add =
        findItem([](QQuickItem *item) { return item->objectName() == u"addNotebook"_s; });
    QVERIFY(add);
    QTest::mouseClick(m_window.get(), Qt::LeftButton, {},
                      add->mapToScene({add->width() / 2, add->height() / 2}).toPoint());
    QTRY_COMPARE(m_sidebar->property("renamingPath").toString(), u"New notebook"_s);
    type(u"Ideas"_s);
    QTest::keyClick(m_window.get(), Qt::Key_Return);
    QTRY_VERIFY(QFileInfo(m_root + u"/Ideas"_s).isDir());
    QVERIFY(!QFileInfo::exists(m_root + u"/New notebook"_s));
}

void TestLists::drop(QQuickItem *target, const QString &format, const QString &data)
{
    // Rows made again after a move are placed a moment later; every row sits below To-dos.
    QTRY_VERIFY(target->mapToScene({0, 0}).y() > 0);
    QMimeData mime;
    mime.setData(format, data.toUtf8());
    const QPointF at = target->mapToScene({target->width() / 2, target->height() / 2});
    QDragEnterEvent enter(at.toPoint(), Qt::MoveAction, &mime, Qt::LeftButton, {});
    QCoreApplication::sendEvent(m_window.get(), &enter);
    QDragMoveEvent move(at.toPoint(), Qt::MoveAction, &mime, Qt::LeftButton, {});
    QCoreApplication::sendEvent(m_window.get(), &move);
    QDropEvent dropped(at, Qt::MoveAction, &mime, Qt::LeftButton, {});
    QCoreApplication::sendEvent(m_window.get(), &dropped);
}

void TestLists::dropsNotesAndNotebooksOnNotebooks()
{
    QTest::failOnWarning(QRegularExpression(u"\\.qml:"_s));
    drop(entry(u"Inbox"_s), u"application/x-jot-note"_s, u"Recipes/Pancakes.md"_s);
    QTRY_VERIFY(QFileInfo::exists(m_root + u"/Inbox/Pancakes.md"_s));

    QTRY_VERIFY(entry(u"Ashfall"_s));
    drop(entry(u"Ashfall"_s), u"application/x-jot-notebook"_s, u"Recipes"_s);
    QTRY_VERIFY(QFileInfo(m_root + u"/Ashfall/Recipes"_s).isDir());

    // All notes is the top of the vault.
    drop(entry(u"All notes"_s), u"application/x-jot-notebook"_s, u"Ashfall/Recipes"_s);
    QTRY_VERIFY(QFileInfo(m_root + u"/Recipes"_s).isDir());
}

void TestLists::noteMenuMakesATodo()
{
    // Looked up each time, since the list makes its rows again when a note changes.
    const auto pancakes = [this] {
        return findItem(
            [](QQuickItem *item) {
                return item->isVisible() &&
                       item->property("path").toString() == u"Recipes/Pancakes.md"_s;
            },
            qobject_cast<QQuickItem *>(m_noteList.get()));
    };
    QVERIFY(pancakes());
    click(pancakes(), Qt::RightButton);
    QTRY_VERIFY(menuEntry(u"Make a to-do"_s));
    click(menuEntry(u"Make a to-do"_s));
    QTRY_VERIFY(m_vault->isTodo(u"Recipes/Pancakes.md"_s));

    // Opened again, it offers the other way.
    QTRY_VERIFY(pancakes());
    click(pancakes(), Qt::RightButton);
    QTRY_VERIFY(menuEntry(u"Not a to-do"_s));
}

void TestLists::notebookMenuMakesANotebookInside()
{
    click(entry(u"Recipes"_s), Qt::RightButton);
    QTRY_VERIFY(menuEntry(u"New notebook inside"_s));
    click(menuEntry(u"New notebook inside"_s));
    QTRY_COMPARE(m_sidebar->property("renamingPath").toString(), u"Recipes/New notebook"_s);
    type(u"Breads"_s);
    QTest::keyClick(m_window.get(), Qt::Key_Return);
    QTRY_VERIFY(QFileInfo(m_root + u"/Recipes/Breads"_s).isDir());
}

void TestLists::trashAsksFirst()
{
    const auto sourdough = [this] {
        return findItem(
            [](QQuickItem *item) {
                return item->isVisible() &&
                       item->property("path").toString() == u"Recipes/Sourdough.md"_s;
            },
            qobject_cast<QQuickItem *>(m_noteList.get()));
    };
    click(sourdough(), Qt::RightButton);
    QTRY_VERIFY(menuEntry(u"Move to trash..."_s));
    click(menuEntry(u"Move to trash..."_s));
    // Nothing goes until the question is answered.
    QVERIFY(QFileInfo::exists(m_root + u"/Recipes/Sourdough.md"_s));
    QQuickItem *confirm = nullptr;
    QTRY_VERIFY((confirm = findItem(
                     [](QQuickItem *item) {
                         return item->isVisible() && item->objectName() == u"confirmTrash"_s;
                     },
                     m_window->contentItem())));
    click(confirm);
    QTRY_VERIFY(!QFileInfo::exists(m_root + u"/Recipes/Sourdough.md"_s));
}

QTEST_MAIN(TestLists)
#include "tst_lists.moc"
