#include "jot/Settings.h"

#include "jot/Identity.h"

#include <QStandardPaths>

using namespace Qt::StringLiterals;

namespace jot {

namespace {

// Never a key without a group: QSettings keeps those in [General], so a group of that
// name is written as [%General] and read back as another.
const QString kVaultPath = u"vault/path"_s;
const QString kTheme = u"appearance/theme"_s;
const QString kNoteSort = u"notes/sort"_s;
const QString kCollapsedNotebooks = u"notes/collapsedNotebooks"_s;
const QString kCheckForUpdates = u"updates/check"_s;
const QString kLastSeenVersion = u"app/lastSeenVersion"_s;

} // namespace

Settings::Settings(const QString &path, QObject *parent)
    : QObject(parent), m_store(path, QSettings::IniFormat)
{
    m_vaultPath = m_store.value(kVaultPath).toString();
    m_theme = m_store.value(kTheme).toString();
    m_noteSort = m_store.value(kNoteSort).toString();
    if (!noteSorts().contains(m_noteSort))
        m_noteSort = noteSorts().first();
    m_collapsedNotebooks = m_store.value(kCollapsedNotebooks).toStringList();
    m_checkForUpdates = m_store.value(kCheckForUpdates, true).toBool();
    m_lastSeenVersion = m_store.value(kLastSeenVersion).toString();
}

QString Settings::defaultPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + u'/' +
           identity::dirName() + u"/settings.ini"_s;
}

QStringList Settings::noteSorts()
{
    return {u"updated"_s, u"title"_s};
}

void Settings::setVaultPath(const QString &path)
{
    if (m_vaultPath == path)
        return;
    m_vaultPath = path;
    m_store.setValue(kVaultPath, path);
    Q_EMIT vaultPathChanged();
}

void Settings::setTheme(const QString &idOrPath)
{
    if (m_theme == idOrPath)
        return;
    m_theme = idOrPath;
    m_store.setValue(kTheme, idOrPath);
    Q_EMIT themeChanged();
}

void Settings::setNoteSort(const QString &sort)
{
    if (m_noteSort == sort || !noteSorts().contains(sort))
        return;
    m_noteSort = sort;
    m_store.setValue(kNoteSort, sort);
    Q_EMIT noteSortChanged();
}

void Settings::setCollapsedNotebooks(const QStringList &paths)
{
    if (m_collapsedNotebooks == paths)
        return;
    m_collapsedNotebooks = paths;
    m_store.setValue(kCollapsedNotebooks, paths);
    Q_EMIT collapsedNotebooksChanged();
}

void Settings::setCheckForUpdates(bool check)
{
    if (m_checkForUpdates == check)
        return;
    m_checkForUpdates = check;
    m_store.setValue(kCheckForUpdates, check);
    Q_EMIT checkForUpdatesChanged();
}

void Settings::setLastSeenVersion(const QString &version)
{
    if (m_lastSeenVersion == version)
        return;
    m_lastSeenVersion = version;
    m_store.setValue(kLastSeenVersion, version);
    Q_EMIT lastSeenVersionChanged();
}

void Settings::reset()
{
    setTheme({});
    setNoteSort(noteSorts().first());
    setCheckForUpdates(true);
}

} // namespace jot
