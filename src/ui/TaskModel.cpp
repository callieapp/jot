#include "TaskModel.h"

#include <QDate>
#include <QDateTime>

using namespace Qt::StringLiterals;

namespace jot {

TaskModel::TaskModel(QObject *parent) : QAbstractListModel(parent) {}

void TaskModel::reload(const VaultIndex *index)
{
    const int before = int(m_rows.size());
    beginResetModel();
    m_rows.clear();
    if (index) {
        for (const NoteInfo &note : index->openTaskNotes())
            m_rows.append({u"note"_s, note.path, note.title, {}, 0, note.due});
        QHash<QString, QString> titles;
        for (const NoteTask &task : index->openTasks()) {
            if (!titles.contains(task.path)) {
                const std::optional<NoteInfo> info = index->note(task.path);
                titles.insert(task.path, info ? info->title : task.path);
            }
            m_rows.append(
                {u"task"_s, task.path, titles.value(task.path), task.text, task.line, {}});
        }
    }
    endResetModel();
    if (before != m_rows.size())
        Q_EMIT countChanged();
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_rows.size())
        return {};
    const Row &row = m_rows.at(index.row());
    switch (role) {
    case KindRole: return row.kind;
    case PathRole: return row.path;
    case TitleRole: return row.title;
    case TaskRole: return row.task;
    case LineRole: return row.line;
    case DueRole: return row.due;
    case GroupRole: return row.kind == u"note" ? tr("Task notes") : row.title;
    case OverdueRole: return isOverdue(row.due, QDate::currentDate());
    }
    return {};
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    return {{KindRole, "kind"}, {PathRole, "path"}, {TitleRole, "title"},     {TaskRole, "task"},
            {LineRole, "line"}, {DueRole, "due"},   {OverdueRole, "overdue"}, {GroupRole, "group"}};
}

} // namespace jot
