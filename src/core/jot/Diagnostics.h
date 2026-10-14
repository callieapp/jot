#pragma once

#include <QString>
#include <QUrl>

namespace jot {

/// What a bug report needs: versions, environment, and the end of each log.
/// Email addresses are masked throughout.
namespace diagnostics {

/// Replaces each distinct email address with <email-1>, <email-2> and so on,
/// and the home folder with ~.
[[nodiscard]] QString redact(const QString &text);

/// Builds the report. `extra` adds lines such as the GUI's platform plugin.
[[nodiscard]] QString collect(const QString &extra = {});

/// A new GitHub issue with `report` filled in, shortened to fit in a URL.
[[nodiscard]] QUrl issueUrl(const QString &report);

} // namespace diagnostics
} // namespace jot
