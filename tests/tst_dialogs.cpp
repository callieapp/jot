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
    QTest::addColumn<QString>("name");
    QTest::addColumn<QVariantMap>("properties");
    const QVariantList shortcuts{
        QVariantMap{{u"keys"_s, QStringList{u"Ctrl+N"_s}}, {u"label"_s, u"New note"_s}},
        QVariantMap{{u"keys"_s, QStringList{u"Ctrl+S"_s}},
                    {u"label"_s, u"Save now"_s},
                    {u"inNote"_s, true}}};
    QTest::newRow("settings") << u"SettingsDialog"_s << QVariantMap{};
    QTest::newRow("shortcuts") << u"KeyHelp"_s << QVariantMap{{u"actions"_s, shortcuts}};
    QTest::newRow("about") << u"AboutDialog"_s << QVariantMap{};
    QTest::newRow("what's new") << u"WhatsNewDialog"_s << QVariantMap{};
    QTest::newRow("update") << u"UpdateDialog"_s << QVariantMap{};
    QTest::newRow("theme colors") << u"ThemeEditor"_s << QVariantMap{};
}

void TestDialogs::opensCleanly()
{
    QFETCH(QString, name);
    QFETCH(QVariantMap, properties);
    QTest::failOnWarning();

    QQuickWindow window;
    window.setColor(ThemeController::instance()->bg());
    window.resize(900, 700);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.loadFromModule("Jot.Ui", name);
    properties.insert(u"parent"_s, QVariant::fromValue(window.contentItem()));
    std::unique_ptr<QObject> dialog(component.createWithInitialProperties(properties));
    QVERIFY2(dialog, qPrintable(component.errorString()));
    QMetaObject::invokeMethod(dialog.get(), "open");
    QTRY_VERIFY(dialog->property("opened").toBool());
    QVERIFY(dialog->property("height").toReal() > 100);
}

QTEST_MAIN(TestDialogs)
#include "tst_dialogs.moc"
