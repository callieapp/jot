#pragma once

#include "jot/VaultIndex.h"

#include <QAbstractListModel>
#include <QQmlEngine>

namespace jot {

class Vault;

/// What points at the note shown: the lines in other notes that link to it, and below
/// them the notes that name it without a link, so a link can be made.
class BacklinksModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Given by Vault")
    Q_PROPERTY(int linked READ linked NOTIFY countsChanged)
    Q_PROPERTY(int unlinked READ unlinked NOTIFY countsChanged)

public:
    enum Role {
        PathRole = Qt::UserRole + 1,
        TitleRole,
        /// The line, or the search's snippet for a mention, without the Markdown fuss.
        TextRole,
        LineRole,
        /// True for a mention that is not a link.
        MentionRole,
    };

    /// The most mentions listed, since past that a search serves better.
    static constexpr int kMentionLimit = 20;

    explicit BacklinksModel(QObject *parent = nullptr);

    /// Reads what reaches `path` in `index`, or empties the list.
    void reload(const Vault *vault, const VaultIndex *index, const QString &path);

    [[nodiscard]] int linked() const { return m_linked; }
    [[nodiscard]] int unlinked() const { return int(m_rows.size()) - m_linked; }

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void countsChanged();

private:
    struct Row
    {
        QString path;
        QString title;
        QString text;
        int line = 0;
        bool mention = false;
    };

    QList<Row> m_rows;
    int m_linked = 0;
};

} // namespace jot
