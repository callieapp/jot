#pragma once

#include <QString>
#include <QVariant>

namespace jot {

/// The formatting a toolbar or shortcut applies to Markdown text, done to the text itself.
/// Each takes the note and a selection, and toggles: applying a style the selection
/// already has takes it away.
namespace markdownedit {

/// One change to make: replace `start` to `end` with `text`, then select
/// `selectionStart` to `selectionEnd` in the changed note.
struct Edit
{
    qsizetype start = 0;
    qsizetype end = 0;
    QString text;
    qsizetype selectionStart = 0;
    qsizetype selectionEnd = 0;

    bool operator==(const Edit &) const = default;
};

/// Styles whole lines take.
enum class LineStyle { Bullet, Numbered, Task, Quote, Heading1, Heading2, Heading3 };

/// Puts `marker` (`**`, `*`, `~~` or a backtick) around the selection, or the word at the
/// cursor, or takes it away. A selection over several lines marks each line.
[[nodiscard]] Edit wrap(const QString &text, qsizetype from, qsizetype to, const QString &marker);

/// Gives every line the selection touches `style`, or takes it away when they all have it.
/// Lists replace one another, so a bullet becomes a task.
[[nodiscard]] Edit styleLines(const QString &text, qsizetype from, qsizetype to, LineStyle style);

/// Makes the selection a link's label, or its target when it is a URL, and leaves the
/// cursor where the other half goes.
[[nodiscard]] Edit link(const QString &text, qsizetype from, qsizetype to);

/// Fences the lines the selection touches as a code block, or unfences them.
[[nodiscard]] Edit codeBlock(const QString &text, qsizetype from, qsizetype to);

/// Sets front matter `key` to `value`, or removes it when `value` is null, keeping the
/// selection on the same text.
[[nodiscard]] Edit setKey(const QString &text, qsizetype from, qsizetype to, const QString &key,
                          const QVariant &value);

/// The smallest change from `before` to `after`, with the selection given in `after`.
[[nodiscard]] Edit difference(const QString &before, const QString &after, qsizetype selectionStart,
                              qsizetype selectionEnd);

} // namespace markdownedit

} // namespace jot
