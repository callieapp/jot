#pragma once

#include "NoteListModel.h"
#include "NotebookModel.h"
#include "SwitcherModel.h"

#include "jot/Vault.h"
#include "jot/VaultIndex.h"
#include "jot/VaultWatcher.h"

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QUrl>

#include <memory>

namespace jot {

class Settings;

/// The `Vault` singleton QML reads: the open vault, its index kept level with the files
/// by a watcher, and the models the sidebar, note list and switcher show.
class VaultController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Vault)
    QML_SINGLETON

    Q_PROPERTY(bool isOpen READ isOpen NOTIFY openChanged)
    Q_PROPERTY(QString root READ root NOTIFY openChanged)
    /// The vault's folder name, for the title bar.
    Q_PROPERTY(QString name READ name NOTIFY openChanged)
    Q_PROPERTY(bool writable READ writable NOTIFY openChanged)
    /// Why the vault last asked for could not be opened, or empty.
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(int noteCount READ noteCount NOTIFY notesChanged)
    Q_PROPERTY(QStringList tags READ tags NOTIFY notesChanged)
    Q_PROPERTY(jot::NotebookModel *notebooks READ notebooks CONSTANT)
    Q_PROPERTY(jot::NoteListModel *notes READ notes CONSTANT)
    Q_PROPERTY(jot::SwitcherModel *switcher READ switcher CONSTANT)
    /// The note shown, by vault path, and what it says.
    Q_PROPERTY(QString current READ current WRITE setCurrent NOTIFY currentChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentChanged)
    Q_PROPERTY(QString currentText READ currentText NOTIFY currentChanged)
    /// Edits not yet written to the note's file.
    Q_PROPERTY(bool unsaved READ unsaved NOTIFY unsavedChanged)
    /// What went wrong with the note shown, such as a save that failed and is being
    /// retried; empty when all is well.
    Q_PROPERTY(QString problem READ problem NOTIFY problemChanged)

public:
    explicit VaultController(Settings &settings, QObject *parent = nullptr);
    ~VaultController() override;

    /// The process-wide instance, which QML also receives. main() makes it first.
    static VaultController *instance();
    static void setInstance(VaultController *controller);
    static VaultController *create(QQmlEngine *, QJSEngine *);

    /// Opens the folder at `path`, making it a vault if it is not one, and remembers it.
    Q_INVOKABLE bool openPath(const QString &path);
    /// As openPath(), for a folder from a folder dialog.
    Q_INVOKABLE bool openFolder(const QUrl &folder);
    /// Opens the vault used last, if there is one.
    bool openLast();
    /// The text of the note shown, as the user typed it; written out shortly after typing
    /// stops, or at once on saveNow().
    Q_INVOKABLE void edit(const QString &text);
    /// Writes any edits waiting, as before switching notes or quitting.
    Q_INVOKABLE void saveNow();
    /// Makes a note in the notebook shown and shows it. Gives its path.
    Q_INVOKABLE QString newNote(const QString &title = {});
    /// Gives the note shown a new title, renaming its file and the links to it.
    Q_INVOKABLE bool renameCurrent(const QString &title);
    /// Moves the note shown, and its attachments, to the system trash.
    Q_INVOKABLE bool trashCurrent();
    /// Follows a link written in the note shown: opens a note, makes one a wikilink names
    /// but nothing holds yet, or opens anything else with the desktop.
    Q_INVOKABLE bool follow(const QString &target, bool wiki);
    /// How long typing must pause before edits are written, in ms.
    void setSaveDelay(int ms)
    {
        m_saveDelay = ms;
        m_saveTimer.setInterval(ms);
    }
    /// Index the database at `path` rather than the default one, as tests do.
    void setIndexPath(const QString &path) { m_indexPath = path; }

    [[nodiscard]] bool isOpen() const { return m_vault != nullptr; }
    [[nodiscard]] QString root() const;
    [[nodiscard]] QString name() const;
    [[nodiscard]] bool writable() const;
    [[nodiscard]] QString error() const { return m_error; }
    [[nodiscard]] int noteCount() const;
    [[nodiscard]] QStringList tags() const;
    [[nodiscard]] NotebookModel *notebooks() { return &m_notebooks; }
    [[nodiscard]] NoteListModel *notes() { return &m_notes; }
    [[nodiscard]] SwitcherModel *switcher() { return &m_switcher; }
    [[nodiscard]] QString current() const { return m_current; }
    void setCurrent(const QString &path);
    [[nodiscard]] QString currentTitle() const;
    [[nodiscard]] QString currentText() const { return m_currentText; }
    [[nodiscard]] bool unsaved() const { return m_unsaved; }
    [[nodiscard]] QString problem() const { return m_problem; }
    [[nodiscard]] const VaultIndex *index() const { return m_index.get(); }

Q_SIGNALS:
    void openChanged();
    void errorChanged();
    void notesChanged();
    void currentChanged();
    void unsavedChanged();
    void problemChanged();

private:
    void close();
    void notesChangedOnDisk(const QStringList &paths);
    void reloadModels();
    void readCurrent();
    void fail(const QString &error);
    void setUnsaved(bool unsaved);
    void setProblem(const QString &problem);
    /// After a rename that did not finish, shows whichever of the two paths now holds
    /// the note, so nothing writes to one that is gone.
    void resyncAfterRename(const QString &from, const QString &to);
    /// Keeps what another program wrote to the note shown beside it, as the user's own
    /// edits are about to replace it.
    void keepConflict(const QString &theirs);
    void scheduleModelReload();

    Settings &m_settings;
    std::unique_ptr<Vault> m_vault;
    std::unique_ptr<VaultIndex> m_index;
    std::unique_ptr<VaultWatcher> m_watcher;
    NotebookModel m_notebooks;
    NoteListModel m_notes;
    SwitcherModel m_switcher;
    QString m_indexPath;
    QString m_error;
    QString m_current;
    QString m_currentText;
    bool m_unsaved = false;
    QString m_problem;
    /// How the note on disk ends its lines and whether it starts with a byte order mark,
    /// which the editor does not keep but the file must.
    bool m_crlf = false;
    bool m_byteOrderMark = false;
    int m_saveDelay = 500;
    QTimer m_saveTimer;
    QTimer m_reloadTimer;
};

} // namespace jot
