#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <QStringView>

namespace jot {

/// What a note's Markdown body holds that the index cares about. Code blocks and code
/// spans are skipped, since a `#include` or `[[x]]` in code is not a tag or a link.
struct NoteScan
{
    struct Link
    {
        /// The target as written, without the label: "Mara", "Mara#Voice", "a%20b.md".
        QString raw;
        /// "link", "wiki" or "embed".
        QString kind;
        /// Counted from 1, in the whole file.
        int line = 0;

        bool operator==(const Link &) const = default;
    };
    struct Task
    {
        int line = 0;
        bool done = false;
        QString text;

        bool operator==(const Task &) const = default;
    };

    /// Inline `#tags`, without the `#`, in the order first seen.
    QStringList tags;
    QList<Link> links;
    QList<Task> tasks;
};

/// Scans `body`, whose first line is line `firstLine` of the file.
[[nodiscard]] NoteScan scanNote(QStringView body, int firstLine = 1);

/// The number of the file's line that `body` starts on, given the whole note.
[[nodiscard]] int bodyFirstLine(QStringView note, QStringView body);

} // namespace jot
