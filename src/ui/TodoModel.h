#pragma once

#include "jot/VaultIndex.h"

#include <QAbstractListModel>
#include <QQmlEngine>

namespace jot {

/// Everything left to do in the vault: notes that are to-dos, soonest due first, then the
/// unticked checkboxes in every note, note by note.
class TodoModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Given by Vault")
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Role {
        /// "note" for a note that is a to-do, "task" for a checkbox in a note.
        KindRole = Qt::UserRole + 1,
        PathRole,
        /// The note's title: what a to-do note says, or where a task is written.
        TitleRole,
        /// A task's own words; empty for a to-do note.
        TaskRole,
        LineRole,
        /// When a to-do note is due, as written, or empty.
        DueRole,
        /// The due date is past.
        OverdueRole,
        /// The heading it is listed under: "To-do notes", or a task's note.
        GroupRole,
    };

    explicit TodoModel(QObject *parent = nullptr);

    void reload(const VaultIndex *index);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void countChanged();

private:
    struct Row
    {
        QString kind;
        QString path;
        QString title;
        QString task;
        int line = 0;
        QString due;
    };

    QList<Row> m_rows;
};

} // namespace jot
