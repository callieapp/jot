#include "SettingsForeign.h"
#include "ThemeController.h"
#include "VaultController.h"

#include "jot/Settings.h"

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QTest>
#include <QtQml/qqmlextensionplugin.h>

Q_IMPORT_QML_PLUGIN(Jot_UiPlugin)

using namespace jot;
using namespace Qt::StringLiterals;

/// Opens each of the title bar's dialogs over a window, so a binding that fails or a
/// singleton that is missing shows up as the warning it would print.
class TestDialogs : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void opensCleanly_data();
    void opensCleanly();

private:
    QTemporaryDir m_dir;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<VaultController> m_vault;
};

void TestDialogs::initTestCase()
{
    QQuickStyle::setStyle(u"Basic"_s);
    // The user's themes and settings are never touched.
    qputenv("XDG_CONFIG_HOME", m_dir.filePath(u"config"_s).toUtf8());
    m_settings = std::make_unique<Settings>(m_dir.filePath(u"settings.ini"_s));
    SettingsForeign::s_instance = m_settings.get();
    m_vault = std::make_unique<VaultController>(*m_settings);
    m_vault->setIndexPath(m_dir.filePath(u"index.sqlite"_s));
    VaultController::setInstance(m_vault.get());
    QVERIFY(m_vault->openPath(m_dir.filePath(u"Notes"_s)));
}

void TestDialogs::opensCleanly_data()
{
    // Each as Main.qml makes it, with what it is given there.
    QTest::addColumn<QString>("dialog");
    QTest::newRow("settings") << u"SettingsDialog {}"_s;
    QTest::newRow("shortcuts") << u"KeyHelp { actions: [{keys: ['Ctrl+N'], label: 'New note'}, "
                                  "{keys: ['Ctrl+S'], label: 'Save now', inNote: true}] }"_s;
    QTest::newRow("about") << u"AboutDialog {}"_s;
    QTest::newRow("what's new") << u"WhatsNewDialog {}"_s;
    QTest::newRow("update") << u"UpdateDialog {}"_s;
    QTest::newRow("theme colors") << u"ThemeEditor {}"_s;
}

void TestDialogs::opensCleanly()
{
    QFETCH(QString, dialog);
    QTest::failOnWarning();

    QQuickWindow window;
    window.setColor(ThemeController::instance()->bg());
    window.resize(900, 700);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    // In a LazyPopup and opened through it, as the app does, since that is what places it.
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(u"import Jot.Ui\nLazyPopup { sourceComponent: %1 }\n"_s.arg(dialog).toUtf8(),
                      QUrl(u"qrc:/test.qml"_s));
    std::unique_ptr<QObject> lazy(component.create());
    QVERIFY2(lazy, qPrintable(component.errorString()));
    qobject_cast<QQuickItem *>(lazy.get())->setParentItem(window.contentItem());
    QMetaObject::invokeMethod(lazy.get(), "open");
    QTRY_VERIFY(lazy->property("opened").toBool());
    auto *popup = lazy->property("popup").value<QObject *>();
    QVERIFY(popup->property("height").toReal() > 100);
    QVERIFY(popup->property("width").toReal() > 100);
}

QTEST_MAIN(TestDialogs)
#include "tst_dialogs.moc"
