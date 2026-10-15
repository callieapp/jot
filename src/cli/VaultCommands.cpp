#include "VaultCommands.h"

#include "jot/Fuzzy.h"

#include <QDateTime>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace jot::cli {

namespace {

QString when(qint64 modified)
{
    return QDateTime::fromMSecsSinceEpoch(modified / 1000000).toString(u"yyyy-MM-dd HH:mm"_s);
}

void print(QTextStream &out, const QJsonValue &value)
{
    out << (value.isArray() ? QJsonDocument(value.toArray()) : QJsonDocument(value.toObject()))
               .toJson(QJsonDocument::Indented);
}

} // namespace

VaultCommands::VaultCommands(const Vault &vault, VaultIndex &index, QTextStream &out,
                             QTextStream &err)
    : m_vault(vault), m_index(index), m_out(out), m_err(err)
{}

int VaultCommands::status(const QString &indexPath, bool json)
{
    const QList<NoteInfo> notes = m_index.notesIn({});
    const int notebooks = int(std::ranges::count_if(m_vault.entries(), &VaultEntry::folder));
    const int tasks = int(m_index.openTasks().size());
    const int todoNotes = int(m_index.openTodoNotes().size());
    if (json) {
        print(m_out, QJsonObject{{u"vault"_s, m_vault.root()},
                                 {u"writable"_s, m_vault.writable()},
                                 {u"notes"_s, int(notes.size())},
                                 {u"notebooks"_s, notebooks},
                                 {u"tags"_s, int(m_index.tags().size())},
                                 {u"openTasks"_s, tasks},
                                 {u"openTodoNotes"_s, todoNotes},
                                 {u"index"_s, indexPath}});
        return 0;
    }
    const auto row = [this](const QString &label, const QString &value) {
        m_out << m_paint.bold(label.leftJustified(12)) << value << "\n";
    };
    row(tr("Vault"), m_vault.root() + (m_vault.writable() ? QString() : tr(" (read only)")));
    row(tr("Notes"),
        (notes.size() == 1 ? tr("1 note") : tr("%1 notes").arg(notes.size())) +
            (notebooks == 1 ? tr(" in 1 notebook") : tr(" in %1 notebooks").arg(notebooks)));
    row(tr("Tags"), QString::number(m_index.tags().size()));
    row(tr("To do"), (todoNotes == 1 ? tr("1 to-do note") : tr("%1 to-do notes").arg(todoNotes)) +
                         u", "_s + (tasks == 1 ? tr("1 task") : tr("%1 tasks").arg(tasks)));
    row(tr("Index"), indexPath);
    return 0;
}

int VaultCommands::list(const QString &notebook, const QString &order, bool json)
{
    if (!notebook.isEmpty() && !QFileInfo(m_vault.absolute(notebook)).isDir()) {
        m_err << tr("jot: no notebook called %1").arg(notebook) << "\n";
        return 1;
    }
    QList<NoteInfo> notes = m_index.notesIn(notebook);
    if (order == u"title")
        std::ranges::sort(notes, [](const NoteInfo &a, const NoteInfo &b) {
            return naturalCompare(a.title, b.title) < 0;
        });
    else
        std::ranges::sort(
            notes, [](const NoteInfo &a, const NoteInfo &b) { return a.modified > b.modified; });
    if (json) {
        QJsonArray array;
        for (const NoteInfo &note : std::as_const(notes))
            array.append(QJsonObject{{u"path"_s, note.path},
                                     {u"title"_s, note.title},
                                     {u"tags"_s, QJsonArray::fromStringList(note.tags)},
                                     {u"modified"_s, when(note.modified)}});
        print(m_out, array);
        return 0;
    }
    for (const NoteInfo &note : std::as_const(notes))
        m_out << m_paint.dim(when(note.modified)) << "  " << m_paint.bold(note.title) << "  "
              << m_paint.dim(note.path) << "\n";
    return 0;
}

int VaultCommands::search(const QString &query, bool json)
{
    if (query.trimmed().isEmpty()) {
        m_err << tr("usage: jot search <words> [tag:name] [in:notebook] [todo:open]") << "\n";
        return 2;
    }
    const QList<SearchHit> hits = m_index.search(query);
    if (json) {
        QJsonArray array;
        for (const SearchHit &hit : hits) {
            QString snippet = hit.snippet;
            snippet.remove(u"<mark>"_s).remove(u"</mark>"_s);
            array.append(QJsonObject{
                {u"path"_s, hit.path}, {u"title"_s, hit.title}, {u"snippet"_s, snippet}});
        }
        print(m_out, array);
        return 0;
    }
    static const QRegularExpression mark(u"<mark>(.*?)</mark>"_s);
    for (const SearchHit &hit : hits) {
        m_out << m_paint.bold(hit.title) << "  " << m_paint.dim(hit.path) << "\n";
        if (hit.snippet.isEmpty())
            continue;
        QString snippet = hit.snippet.simplified();
        QString shown;
        qsizetype from = 0;
        for (const QRegularExpressionMatch &m : mark.globalMatch(snippet)) {
            shown += snippet.mid(from, m.capturedStart() - from) + m_paint.accent(m.captured(1));
            from = m.capturedEnd();
        }
        m_out << "    " << shown + snippet.mid(from) << "\n";
    }
    if (hits.isEmpty())
        m_err << tr("Nothing matches.") << "\n";
    return 0;
}

int VaultCommands::create(const QString &title, const QString &notebook, const QString &body,
                          bool json)
{
    if (title.trimmed().isEmpty()) {
        m_err << tr("usage: jot new <title> [--notebook name]") << "\n";
        return 2;
    }
    QString error;
    const std::optional<QString> path =
        m_vault.createNote(notebook, title.trimmed(), QDateTime::currentDateTimeUtc(), &error);
    if (!path) {
        m_err << tr("jot: could not make the note: %1").arg(error) << "\n";
        return 1;
    }
    if (!body.isEmpty()) {
        QString text = notefile::read(m_vault.absolute(*path), &error).value_or(QString());
        text += body.endsWith(u'\n') ? body : body + u'\n';
        if (!notefile::write(m_vault.absolute(*path), text, &error)) {
            m_err << tr("jot: could not write the note: %1").arg(error) << "\n";
            return 1;
        }
    }
    m_index.update({*path});
    if (json)
        print(m_out, QJsonObject{{u"path"_s, *path}, {u"file"_s, m_vault.absolute(*path)}});
    else
        m_out << *path << "\n";
    return 0;
}

QString VaultCommands::find(const QString &name) const
{
    if (name.isEmpty())
        return {};
    // A path or a name as a link would write it; else the closest title.
    const QString linked = m_index.resolve(name, {});
    if (linked.endsWith(u".md"_s))
        return linked;
    QString best;
    int bestScore = 0;
    for (const NoteInfo &note : m_index.notesIn({})) {
        for (const QString &candidate : QStringList{note.title} + note.aliases) {
            const std::optional<int> score = fuzzyScore(name, candidate);
            if (score && (best.isEmpty() || *score > bestScore)) {
                best = note.path;
                bestScore = *score;
            }
        }
    }
    return best;
}

int VaultCommands::show(const QString &name)
{
    const QString path = find(name);
    if (path.isEmpty()) {
        m_err << tr("jot: no note called %1").arg(name) << "\n";
        return 1;
    }
    QString error;
    const std::optional<QString> text = notefile::read(m_vault.absolute(path), &error);
    if (!text) {
        m_err << tr("jot: could not read %1: %2").arg(path, error) << "\n";
        return 1;
    }
    m_out << *text;
    return 0;
}

int VaultCommands::todo(bool json)
{
    const QList<NoteInfo> notes = m_index.openTodoNotes();
    const QList<NoteTask> tasks = m_index.openTasks();
    if (json) {
        QJsonArray array;
        for (const NoteInfo &note : notes)
            array.append(QJsonObject{{u"kind"_s, u"note"_s},
                                     {u"path"_s, note.path},
                                     {u"title"_s, note.title},
                                     {u"due"_s, note.due}});
        for (const NoteTask &task : tasks)
            array.append(QJsonObject{{u"kind"_s, u"task"_s},
                                     {u"path"_s, task.path},
                                     {u"line"_s, task.line},
                                     {u"text"_s, task.text}});
        print(m_out, array);
        return 0;
    }
    for (const NoteInfo &note : notes) {
        const bool overdue =
            !note.due.isEmpty() &&
            QDate::fromString(note.due.left(10), Qt::ISODate) < QDate::currentDate();
        m_out << "[ ] " << m_paint.bold(note.title)
              << (note.due.isEmpty() ? QString()
                                     : u"  "_s + (overdue ? m_paint.danger(note.due.left(10))
                                                          : m_paint.dim(note.due.left(10))))
              << "\n";
    }
    QString last;
    for (const NoteTask &task : tasks) {
        if (task.path != last) {
            m_out << m_paint.dim(task.path) << "\n";
            last = task.path;
        }
        m_out << "  [ ] " << task.text << m_paint.dim(u"  :%1"_s.arg(task.line)) << "\n";
    }
    if (notes.isEmpty() && tasks.isEmpty())
        m_err << tr("Nothing left to do.") << "\n";
    return 0;
}

} // namespace jot::cli
