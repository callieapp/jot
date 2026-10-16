#include "jot/CodeColors.h"

#include "jot/Color.h"

#include <algorithm>
#include <cmath>

namespace jot {

CodeColors codeColors(const ThemeSpec &theme)
{
    const color::Oklch accent = color::toOklch(theme.colors.accent);
    // A grey accent has no hue to turn, so the others get a little color of their own.
    const double chroma = std::max(accent.chroma, 0.08);
    const auto turned = [&](double degrees) {
        return color::fromOklch(
            {accent.lightness, chroma, std::fmod(accent.hue + degrees + 360.0, 360.0)});
    };
    const auto pick = [](const QColor &set, const QColor &worked) {
        return set.isValid() ? set : worked;
    };
    const ThemeSpec::Editor &editor = theme.editor;
    return {pick(editor.keyword, theme.colors.accent), pick(editor.string, turned(150)),
            pick(editor.number, turned(60)), pick(editor.type, turned(270)),
            pick(editor.comment, theme.colors.textFaint)};
}

} // namespace jot
