#pragma once

#include <QList>
#include <QStringView>

#include <optional>

namespace jot {

/// How well `pattern` matches `text` for a quick switcher: its characters must appear in
/// order, ignoring case, and matches at the start of words or in a run score higher.
/// Higher is better; nothing means no match. `positions` receives the matched characters.
[[nodiscard]] std::optional<int> fuzzyScore(QStringView pattern, QStringView text,
                                            QList<qsizetype> *positions = nullptr);

} // namespace jot
