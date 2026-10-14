#pragma once

#include <QString>

/// Who this build of Jot is. A development build (JOT_DEVEL) has its own
/// app id, folders and keyring entries, so it runs beside an installed Jot
/// without sharing its accounts, cache or settings.
namespace jot::identity {

/// What a build is called, worked out from whether it is a development build.
struct Names
{
    /// "org.callieapp.Jot", or "org.callieapp.Jot.Devel".
    QString appId;
    /// The folder under each XDG directory: "jot" or "jot-devel".
    QString dirName;
    /// "Jot" or "Jot Devel", for what the desktop shows.
    QString displayName;
};
[[nodiscard]] Names namesFor(bool devel);

/// This build's names.
[[nodiscard]] QString appId();
[[nodiscard]] QString dirName();
[[nodiscard]] QString displayName();
[[nodiscard]] bool isDevel();

} // namespace jot::identity
