#include "jot/FrontMatter.h"

namespace jot {

namespace {

// The line starting at `from`, without its line break, and where the next line starts.
QStringView lineAt(QStringView text, qsizetype from, qsizetype *next)
{
    qsizetype end = text.indexOf(u'\n', from);
    *next = end < 0 ? text.size() : end + 1;
    if (end < 0)
        end = text.size();
    if (end > from && text[end - 1] == u'\r')
        --end;
    return text.sliced(from, end - from);
}

bool isFence(QStringView line, bool closing)
{
    const QStringView trimmed = line.trimmed();
    return trimmed == u"---" || (closing && trimmed == u"...");
}

} // namespace

NoteParts splitFrontMatter(QStringView note)
{
    qsizetype next = 0;
    if (!isFence(lineAt(note, 0, &next), false) || !note.startsWith(u"---"))
        return {{}, note, false};

    const qsizetype yamlStart = next;
    while (next < note.size()) {
        const qsizetype lineStart = next;
        if (isFence(lineAt(note, lineStart, &next), true))
            return {note.sliced(yamlStart, lineStart - yamlStart), note.sliced(next), true};
    }
    return {{}, note, false};
}

} // namespace jot
