#include "SettingsForeign.h"
#include "ThemeController.h"
#include "VaultController.h"

#include "jot/Settings.h"

#include <QDirIterator>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTest>
#include <QtQml/qqmlextensionplugin.h>

#include <functional>

Q_IMPORT_QML_PLUGIN(Jot_UiPlugin)

using namespace jot;
using namespace Qt::StringLiterals;

/// Drives the sidebar in a window: making and renaming notebooks by hand.
class TestSidebar : public QObject
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

private:
    /// The sidebar entry labelled `label`, among those shown.
    QQuickItem *entry(const QString &label) const;
    QQuickItem *findItem(const std::function<bool(QQuickItem *)> &match,
                         QQuickItem *from = nullptr) const;
    void type(const QString &text);

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<VaultController> m_vault;
    std::unique_ptr<QQmlEngine> m_engine;
    std::unique_ptr<QQuickWindow> m_window;
    std::unique_ptr<QObject> m_sidebar;
    QString m_root;
};

void TestSidebar::init()
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
    VaultController::setInstance(m_vault.get());
    QVERIFY(m_vault->openPath(m_root));

    m_engine = std::make_unique<QQmlEngine>();
    QQmlComponent component(m_engine.get());
    component.loadFromModule("Jot.Ui", "Sidebar");
    m_sidebar.reset(component.create());
    QVERIFY2(m_sidebar, qPrintable(component.errorString()));
    m_window = std::make_unique<QQuickWindow>();
    m_window->resize(320, 700);
    auto *item = qobject_cast<QQuickItem *>(m_sidebar.get());
    item->setParentItem(m_window->contentItem());
    item->setSize(m_window->size());
    m_window->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_window.get()));
}

void TestSidebar::cleanup()
{
    m_sidebar.reset();
    m_window.reset();
    m_engine.reset();
    VaultController::setInstance(nullptr);
    m_vault.reset();
    SettingsForeign::s_instance = nullptr;
}

QQuickItem *TestSidebar::findItem(const std::function<bool(QQuickItem *)> &match,
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

QQuickItem *TestSidebar::entry(const QString &label) const
{
    return findItem([&label](QQuickItem *item) {
        return item->isVisible() && item->property("label").toString() == label;
    });
}

void TestSidebar::type(const QString &text)
{
    for (const QChar c : text)
        QTest::keyClick(m_window.get(), c.toLatin1());
}

void TestSidebar::leavesRoomUnderTheTitleBar()
{
    QQuickItem *todos = entry(u"To-dos"_s);
    QVERIFY(todos);
    QVERIFY(todos->mapToScene({0, 0}).y() >= ThemeController::instance()->space3());
}

void TestSidebar::renamesANotebookByDoubleClick()
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

void TestSidebar::escapeKeepsTheOldName()
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

void TestSidebar::makesANotebookReadyToName()
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

QTEST_MAIN(TestSidebar)
#include "tst_sidebar.moc"
