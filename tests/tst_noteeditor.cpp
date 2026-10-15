#include "ThemeController.h"
#include "VaultController.h"

#include "jot/Settings.h"

#include <QDir>
#include <QDirIterator>
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

/// Drives the real editor in a window, since what matters is the text being drawn and
/// saved, which the models alone cannot show.
class TestNoteEditor : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { QQuickStyle::setStyle(u"Basic"_s); }
    void init();
    void cleanup();

    void drawsTheNoteAndKeepsDrawingItAcrossSwitches();
    void typingIsDrawnAndSaved();
    void typingKeepsWindowsLineEndings();

private:
    /// Pixels in the body's area that are not the background, as a measure of text drawn.
    int inkBelowTitle();
    QQuickItem *find(const QString &name, QQuickItem *from = nullptr) const;

    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<VaultController> m_vault;
    std::unique_ptr<QQmlEngine> m_engine;
    std::unique_ptr<QQuickWindow> m_window;
    std::unique_ptr<QObject> m_editor;
    QString m_root;
};

void TestNoteEditor::init()
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
    m_vault->setSaveDelay(50);
    VaultController::setInstance(m_vault.get());
    QVERIFY(m_vault->openPath(m_root));
    m_vault->setCurrent(u"Ashfall/Characters/Mara Vey.md"_s);

    m_engine = std::make_unique<QQmlEngine>();
    QQmlComponent component(m_engine.get());
    component.loadFromModule("Jot.Ui", "NoteEditor");
    m_editor.reset(component.create());
    QVERIFY2(m_editor, qPrintable(component.errorString()));
    m_window = std::make_unique<QQuickWindow>();
    m_window->setColor(ThemeController::instance()->bg());
    m_window->resize(900, 700);
    auto *item = qobject_cast<QQuickItem *>(m_editor.get());
    item->setParentItem(m_window->contentItem());
    item->setSize(m_window->size());
    m_window->show();
    QVERIFY(QTest::qWaitForWindowExposed(m_window.get()));
}

void TestNoteEditor::cleanup()
{
    m_editor.reset();
    m_window.reset();
    m_engine.reset();
    VaultController::setInstance(nullptr);
    m_vault.reset();
}

int TestNoteEditor::inkBelowTitle()
{
    const QImage image = m_window->grabWindow();
    const QColor background = ThemeController::instance()->bg();
    int ink = 0;
    for (int y = 140; y < image.height(); y += 2) {
        for (int x = 0; x < image.width(); x += 2) {
            const QColor pixel = image.pixelColor(x, y);
            if (std::abs(pixel.red() - background.red()) +
                    std::abs(pixel.green() - background.green()) +
                    std::abs(pixel.blue() - background.blue()) >
                90)
                ++ink;
        }
    }
    return ink;
}

QQuickItem *TestNoteEditor::find(const QString &name, QQuickItem *from) const
{
    if (!from)
        from = qobject_cast<QQuickItem *>(m_editor.get());
    for (QQuickItem *child : from->childItems()) {
        if (child->property("objectName").toString() == name)
            return child;
        if (QQuickItem *found = find(name, child))
            return found;
    }
    return nullptr;
}

void TestNoteEditor::drawsTheNoteAndKeepsDrawingItAcrossSwitches()
{
    QTRY_VERIFY_WITH_TIMEOUT(inkBelowTitle() > 500, 3000);
    m_vault->setCurrent(u"Ashfall/Pitch.md"_s);
    QTRY_VERIFY_WITH_TIMEOUT(inkBelowTitle() > 200, 3000);
    m_vault->setCurrent(u"Recipes/Pancakes.md"_s);
    QTRY_VERIFY_WITH_TIMEOUT(inkBelowTitle() > 100, 3000);
}

void TestNoteEditor::typingIsDrawnAndSaved()
{
    m_vault->setCurrent(u"Welcome.md"_s);
    QQuickItem *body = find(u"body"_s);
    QVERIFY(body);
    body->forceActiveFocus();
    body->setProperty("cursorPosition", body->property("length"));
    for (const QChar c : u"\n# A heading typed here\nAnd **bold** words."_s) {
        if (c == u'\n')
            QTest::keyClick(m_window.get(), Qt::Key_Return);
        else
            QTest::keyClick(m_window.get(), c.toLatin1());
    }
    QTRY_VERIFY_WITH_TIMEOUT(m_vault->currentText().contains(u"**bold** words."_s), 2000);
    QTRY_VERIFY_WITH_TIMEOUT(!m_vault->unsaved(), 2000);
    QTRY_VERIFY_WITH_TIMEOUT(inkBelowTitle() > 400, 3000);

    QString error;
    QVERIFY(notefile::read(m_root + u"/Welcome.md"_s, &error)->endsWith(u"And **bold** words."_s));
}

void TestNoteEditor::typingKeepsWindowsLineEndings()
{
    QFile file(m_root + u"/Windows.md"_s);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("one\r\ntwo\r\n");
    file.close();
    // Written before the window opened, so the watcher may not have told the index yet.
    QTRY_VERIFY_WITH_TIMEOUT(m_vault->index()->note(u"Windows.md"_s), 3000);
    m_vault->setCurrent(u"Windows.md"_s);
    QQuickItem *body = find(u"body"_s);
    body->forceActiveFocus();
    body->setProperty("cursorPosition", 3);
    QTest::keyClick(m_window.get(), '!');
    QTRY_VERIFY_WITH_TIMEOUT(!m_vault->unsaved(), 2000);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), QByteArray("one!\r\ntwo\r\n"));
}

QTEST_MAIN(TestNoteEditor)
#include "tst_noteeditor.moc"
