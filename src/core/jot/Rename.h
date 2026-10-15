#pragma once

#include "jot/Vault.h"
#include "jot/VaultIndex.h"

#include <QStringList>

#include <functional>
#include <optional>

namespace jot {

/// Called with each note about to be written, so a watcher can expect the write.
using BeforeWrite = std::function<void(const QString &path, const QString &text)>;

/// Moves the note at `from` to `to`, with its attachments, and rewrites the links in other
/// notes that reached it so they still do, in the style each was written in. Gives the
/// notes it rewrote. The index is brought up to date with all of them.
std::optional<QStringList> renameNote(const Vault &vault, VaultIndex &index, const QString &from,
                                      const QString &to, QString *error,
                                      const BeforeWrite &beforeWrite = {});

/// The target to write for a link that reached `from` from `source` and should now reach `to`:
/// a name or vault path for a wikilink, as `raw` was; a path from the source's folder for a
/// Markdown link. Any `#heading` stays.
[[nodiscard]] QString retarget(const QString &raw, bool wiki, const QString &source,
                               const QString &to);

} // namespace jot
