#include "VaultController.h"

#include "jot/FrontMatter.h"
#include "jot/Logging.h"
#include "jot/Rename.h"
#include "jot/Settings.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

using namespace Qt::StringLiterals;

namespace jot {

namespace {

VaultController *s_instance = nullptr;

} // namespace

VaultController::VaultController(Settings &settings, QObject *parent)
    : QObject(parent), m_settings(settings), m_notebooks(settings)
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(500);
    connect(&m_saveTimer, &QTimer::timeout, this, &VaultController::saveNow);
    // Saving changes the list's order and titles; a burst of saves reloads it once.
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(800);
    connect(&m_reloadTimer, &QTimer::timeout, this, &VaultController::reloadModels);
    m_notes.setOrder(settings.noteSort());
    connect(&m_notes, &NoteListModel::orderChanged, this,
            [this] { m_settings.setNoteSort(m_notes.order()); });
}

VaultController::~VaultController()
{
    saveNow();
    if (s_instance == this)
        s_instance = nullptr;
}

VaultController *VaultController::instance()
{
    return s_instance;
}

void VaultController::setInstance(VaultController *controller)
{
    s_instance = controller;
}

VaultController *VaultController::create(QQmlEngine *, QJSEngine *)
{
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

bool VaultController::openLast()
{
    const QString path = m_settings.vaultPath();
    return !path.isEmpty() && QFileInfo(path).isDir() && openPath(path);
}

bool VaultController::openFolder(const QUrl &folder)
{
    return openPath(folder.toLocalFile());
}

bool VaultController::openPath(const QString &path)
{
    QString error;
    const std::optional<Vault> created = Vault::create(QDir(path).absolutePath(), &error);
    if (!created) {
        fail(error);
        return false;
    }
    auto vault = std::make_unique<Vault>(*created);
    auto index = std::make_unique<VaultIndex>(*vault, m_indexPath);
    if (!index->open(&error)) {
        fail(tr("Could not open the index for %1: %2").arg(path, error));
        return false;
    }

    close();
    m_vault = std::move(vault);
    m_index = std::move(index);
    m_index->refresh();
    m_watcher = std::make_unique<VaultWatcher>(*m_vault);
    connect(m_watcher.get(), &VaultWatcher::changed, this, &VaultController::notesChangedOnDisk);
    connect(m_watcher.get(), &VaultWatcher::foldersChanged, this, &VaultController::reloadModels);
    m_watcher->start();
    m_settings.setVaultPath(m_vault->root());
    qCInfo(lcVault) << "opened" << m_vault->root() << "with" << m_index->noteCount() << "notes";

    if (!m_error.isEmpty()) {
        m_error.clear();
        Q_EMIT errorChanged();
    }
    reloadModels();
    Q_EMIT openChanged();
    return true;
}

void VaultController::close()
{
    saveNow();
    m_watcher.reset();
    m_index.reset();
    m_vault.reset();
    m_current.clear();
    m_currentText.clear();
}

void VaultController::fail(const QString &error)
{
    qCWarning(lcVault) << "could not open the vault:" << error;
    m_error = error;
    Q_EMIT errorChanged();
}

void VaultController::notesChangedOnDisk(const QStringList &paths)
{
    if (paths.contains(m_current)) {
        if (m_unsaved) {
            QString error;
            if (const std::optional<QString> theirs =
                    notefile::read(m_vault->absolute(m_current), &error))
                keepConflict(*theirs);
        } else {
            readCurrent();
        }
    }
    m_index->update(paths);
    reloadModels();
}

void VaultController::keepConflict(const QString &theirs)
{
    const QString stem = Vault::titleFromPath(m_current);
    const QString folder = m_current.contains(u'/') ? m_current.section(u'/', 0, -2) : QString();
    const QString title =
        tr("%1 (conflict %2)")
            .arg(stem, QDateTime::currentDateTime().toString(u"yyyy-MM-dd HHmm"_s));
    const QString path = m_vault->freePath(folder, title);
    QString error;
    if (notefile::write(m_vault->absolute(path), theirs, &error))
        qCWarning(lcVault) << m_current
                           << "changed on disk while being edited; kept the other "
                              "version as"
                           << path;
    else
        qCWarning(lcVault) << "could not keep the other version of" << m_current << error;
}

void VaultController::setUnsaved(bool unsaved)
{
    if (m_unsaved == unsaved)
        return;
    m_unsaved = unsaved;
    Q_EMIT unsavedChanged();
}

void VaultController::edit(const QString &text)
{
    if (!m_vault || m_current.isEmpty() || text == m_currentText)
        return;
    m_currentText = text;
    setUnsaved(true);
    m_saveTimer.start();
}

void VaultController::saveNow()
{
    m_saveTimer.stop();
    if (!m_unsaved || !m_vault || m_current.isEmpty())
        return;
    if (!m_vault->writable()) {
        qCWarning(lcVault) << "not saving" << m_current << "in a vault from a newer Jot";
        return;
    }
    QString error;
    if (m_watcher)
        m_watcher->expectWrite(m_current, m_currentText);
    if (!notefile::write(m_vault->absolute(m_current), m_currentText, &error)) {
        qCWarning(lcVault) << "could not save" << m_current << error;
        return;
    }
    setUnsaved(false);
    m_index->update({m_current});
    scheduleModelReload();
}

void VaultController::scheduleModelReload()
{
    m_reloadTimer.start();
}

QString VaultController::newNote(const QString &title)
{
    if (!m_vault)
        return {};
    saveNow();
    // In the notebook shown; a tag or "All notes" puts it at the top of the vault.
    const QString folder = m_notes.tag().isEmpty() ? m_notes.notebook() : QString();
    QString error;
    const std::optional<QString> path = m_vault->createNote(
        folder, title.isEmpty() ? tr("Untitled") : title, QDateTime::currentDateTimeUtc(), &error);
    if (!path) {
        qCWarning(lcVault) << "could not make a note:" << error;
        return {};
    }
    m_index->update({*path});
    reloadModels();
    setCurrent(*path);
    return *path;
}

bool VaultController::renameCurrent(const QString &title)
{
    const QString wanted = title.trimmed();
    if (!m_vault || m_current.isEmpty() || wanted.isEmpty() || wanted == currentTitle())
        return false;
    saveNow();
    const QString folder = m_current.contains(u'/') ? m_current.section(u'/', 0, -2) : QString();
    QString to = folder.isEmpty() ? Vault::fileNameFor(wanted) + u".md"_s
                                  : folder + u'/' + Vault::fileNameFor(wanted) + u".md"_s;
    if (to.compare(m_current, Qt::CaseSensitive) != 0 && QFileInfo::exists(m_vault->absolute(to)))
        to = m_vault->freePath(folder, wanted);

    QString error;
    if (to != m_current) {
        const auto expect = [this](const QString &path, const QString &text) {
            if (m_watcher)
                m_watcher->expectWrite(path, text);
        };
        if (!renameNote(*m_vault, *m_index, m_current, to, &error, expect)) {
            qCWarning(lcVault) << "could not rename" << m_current << error;
            return false;
        }
        m_current = to;
        readCurrent();
    }
    // The title key holds what the file name cannot; when the name can, it goes.
    const QVariantMap meta = readFrontMatter(splitFrontMatter(m_currentText).frontMatter);
    const bool fits = Vault::titleFromPath(to) == wanted;
    if (!fits || meta.contains(u"title"_s)) {
        const QString text =
            setFrontMatterKey(m_currentText, u"title"_s, fits ? QVariant() : QVariant(wanted));
        if (text != m_currentText) {
            m_currentText = text;
            setUnsaved(true);
            saveNow();
            Q_EMIT currentChanged();
        }
    }
    m_index->update({m_current});
    reloadModels();
    return true;
}

bool VaultController::trashCurrent()
{
    if (!m_vault || m_current.isEmpty())
        return false;
    saveNow();
    QString error;
    const QString path = m_current;
    if (!m_vault->trashNote(path, &error)) {
        qCWarning(lcVault) << "could not move" << path << "to the trash:" << error;
        return false;
    }
    m_index->update({path});
    setCurrent({});
    reloadModels();
    return true;
}

bool VaultController::follow(const QString &target, bool wiki)
{
    if (!m_vault)
        return false;
    static const QRegularExpression scheme(u"^[A-Za-z][A-Za-z0-9+.-]*:"_s);
    if (!wiki && scheme.match(target).hasMatch())
        return QDesktopServices::openUrl(QUrl(target));
    const QString path = m_index->resolve(target, m_current);
    if (path.endsWith(u".md"_s)) {
        setCurrent(path);
        return true;
    }
    if (!path.isEmpty())
        return QDesktopServices::openUrl(QUrl::fromLocalFile(m_vault->absolute(path)));
    if (!wiki)
        return false;
    // A wikilink to nothing yet makes the note, beside the one that links to it.
    const QString name = target.section(u'#', 0, 0).section(u'/', -1).trimmed();
    if (name.isEmpty())
        return false;
    const QString folder = m_current.contains(u'/') ? m_current.section(u'/', 0, -2) : QString();
    saveNow();
    QString error;
    const std::optional<QString> created =
        m_vault->createNote(folder, name, QDateTime::currentDateTimeUtc(), &error);
    if (!created)
        return false;
    m_index->update({*created});
    reloadModels();
    setCurrent(*created);
    return true;
}

void VaultController::reloadModels()
{
    if (m_index) {
        m_notebooks.reload(m_vault.get(), m_index->noteCounts(), m_index->noteCount());
        m_notes.reload(m_index.get());
        m_switcher.reload(m_index.get());
    } else {
        m_notebooks.reload(nullptr, {}, 0);
        m_notes.reload(nullptr);
        m_switcher.reload(nullptr);
    }
    Q_EMIT notesChanged();
}

QString VaultController::root() const
{
    return m_vault ? m_vault->root() : QString();
}

QString VaultController::name() const
{
    return m_vault ? QFileInfo(m_vault->root()).fileName() : QString();
}

bool VaultController::writable() const
{
    return m_vault && m_vault->writable();
}

int VaultController::noteCount() const
{
    return m_index ? m_index->noteCount() : 0;
}

QStringList VaultController::tags() const
{
    return m_index ? m_index->tags() : QStringList();
}

void VaultController::setCurrent(const QString &path)
{
    if (m_current == path)
        return;
    saveNow();
    m_current = path;
    readCurrent();
}

QString VaultController::currentTitle() const
{
    if (!m_index || m_current.isEmpty())
        return {};
    const std::optional<NoteInfo> info = m_index->note(m_current);
    return info ? info->title : Vault::titleFromPath(m_current);
}

void VaultController::readCurrent()
{
    m_saveTimer.stop();
    setUnsaved(false);
    m_currentText.clear();
    if (m_vault && !m_current.isEmpty()) {
        QString error;
        if (const std::optional<QString> text =
                notefile::read(m_vault->absolute(m_current), &error))
            m_currentText = *text;
        else
            qCWarning(lcVault) << "could not read" << m_current << error;
    }
    Q_EMIT currentChanged();
}

} // namespace jot
