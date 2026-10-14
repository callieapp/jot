#pragma once

#include <QStringView>

namespace jot {

/// A note split into its YAML front matter and its Markdown body. Both views point into
/// the text that was split, which must outlive them.
struct NoteParts
{
    /// The YAML between the fences, without them. Empty when the note has none.
    QStringView frontMatter;
    /// Everything after the closing fence, or the whole note without front matter.
    QStringView body;
    bool hasFrontMatter = false;
};

/// Front matter is a `---` line at the very start of the note, closed by a `---` or `...`
/// line. Anything else, including an unclosed fence, is all body, so no text is lost.
NoteParts splitFrontMatter(QStringView note);

} // namespace jot
