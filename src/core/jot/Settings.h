#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>

namespace jot {

/// The user's preferences, kept as INI in the config folder. Every setter saves at once
/// and signals only on a change. Settings that belong to a vault rather than to this
/// computer live in the vault's `.jot/` instead.
class Settings : public QObject
{
    Q_OBJECT
    /// The folder of the vault open last, empty before one is chosen.
    Q_PROPERTY(QString vaultPath READ vaultPath WRITE setVaultPath NOTIFY vaultPathChanged)
    /// A built-in theme id or a theme file path; empty is the default theme.
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    /// How the note list is ordered: "updated" or "title".
    Q_PROPERTY(QString noteSort READ noteSort WRITE setNoteSort NOTIFY noteSortChanged)
    /// Notebooks folded shut in the sidebar, by vault path.
    Q_PROPERTY(QStringList collapsedNotebooks READ collapsedNotebooks WRITE setCollapsedNotebooks
                   NOTIFY collapsedNotebooksChanged)
    /// Looks for newer releases once a day.
    Q_PROPERTY(bool checkForUpdates READ checkForUpdates WRITE setCheckForUpdates NOTIFY
                   checkForUpdatesChanged)
    /// The sidebar's and the note list's widths in pixels, as dragged; 0 is the theme's own.
    Q_PROPERTY(int sidebarWidth READ sidebarWidth WRITE setSidebarWidth NOTIFY sidebarWidthChanged)
    Q_PROPERTY(
        int noteListWidth READ noteListWidth WRITE setNoteListWidth NOTIFY noteListWidthChanged)
    /// The font notes are written in: "" for the theme's, "serif" or "mono".
    Q_PROPERTY(QString noteFont READ noteFont WRITE setNoteFont NOTIFY noteFontChanged)
    /// Notes show their Markdown as written rather than styled.
    Q_PROPERTY(bool sourceMode READ sourceMode WRITE setSourceMode NOTIFY sourceModeChanged)
    /// The version that ran last, so a new one can say what changed.
    Q_PROPERTY(QString lastSeenVersion READ lastSeenVersion WRITE setLastSeenVersion NOTIFY
                   lastSeenVersionChanged)

public:
    explicit Settings(const QString &path, QObject *parent = nullptr);

    /// $XDG_CONFIG_HOME/jot/settings.ini
    [[nodiscard]] static QString defaultPath();
    [[nodiscard]] static QStringList noteSorts();

    [[nodiscard]] QString vaultPath() const { return m_vaultPath; }
    void setVaultPath(const QString &path);
    [[nodiscard]] QString theme() const { return m_theme; }
    void setTheme(const QString &idOrPath);
    [[nodiscard]] QString noteSort() const { return m_noteSort; }
    void setNoteSort(const QString &sort);
    [[nodiscard]] QStringList collapsedNotebooks() const { return m_collapsedNotebooks; }
    void setCollapsedNotebooks(const QStringList &paths);
    [[nodiscard]] bool checkForUpdates() const { return m_checkForUpdates; }
    void setCheckForUpdates(bool check);
    [[nodiscard]] int sidebarWidth() const { return m_sidebarWidth; }
    /// Kept between 180 and 480, or 0 for the theme's width.
    void setSidebarWidth(int width);
    [[nodiscard]] int noteListWidth() const { return m_noteListWidth; }
    /// Kept between 220 and 640, or 0 for the theme's width.
    void setNoteListWidth(int width);
    [[nodiscard]] static QStringList noteFonts();
    [[nodiscard]] QString noteFont() const { return m_noteFont; }
    void setNoteFont(const QString &font);
    [[nodiscard]] bool sourceMode() const { return m_sourceMode; }
    void setSourceMode(bool source);
    [[nodiscard]] QString lastSeenVersion() const { return m_lastSeenVersion; }
    void setLastSeenVersion(const QString &version);

    /// Puts every choice back to its default. The vault stays open, and which
    /// notebooks are folded is kept, since neither is a preference.
    Q_INVOKABLE void reset();

Q_SIGNALS:
    void vaultPathChanged();
    void themeChanged();
    void noteSortChanged();
    void collapsedNotebooksChanged();
    void checkForUpdatesChanged();
    void noteFontChanged();
    void sidebarWidthChanged();
    void noteListWidthChanged();
    void sourceModeChanged();
    void lastSeenVersionChanged();

private:
    QSettings m_store;
    QString m_vaultPath;
    QString m_theme;
    QString m_noteSort;
    QStringList m_collapsedNotebooks;
    bool m_checkForUpdates = true;
    QString m_noteFont;
    int m_sidebarWidth = 0;
    int m_noteListWidth = 0;
    bool m_sourceMode = false;
    QString m_lastSeenVersion;
};

} // namespace jot
