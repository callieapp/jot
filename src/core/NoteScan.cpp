#include "jot/NoteScan.h"

#include <QRegularExpression>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace jot {

namespace {

/// `line` with its code spans blanked out, so their contents match nothing.
QString withoutCodeSpans(const QString &line)
{
    QString result = line;
    qsizetype i = 0;
    while (i < result.size()) {
        if (result[i] != u'`') {
            ++i;
            continue;
        }
        qsizetype ticks = 0;
        while (i + ticks < result.size() && result[i + ticks] == u'`')
            ++ticks;
        const QString fence(ticks, u'`');
        const qsizetype close = result.indexOf(fence, i + ticks);
        if (close < 0)
            break;
        for (qsizetype j = i; j < close + ticks; ++j)
            result[j] = u' ';
        i = close + ticks;
    }
    return result;
}

bool isUrl(const QString &target)
{
    static const QRegularExpression scheme(u"^[A-Za-z][A-Za-z0-9+.-]*:"_s);
    return scheme.match(target).hasMatch();
}

} // namespace

NoteScan scanNote(QStringView body, int firstLine)
{
    static const QRegularExpression wiki(uR"((!?)\[\[([^\[\]|]*?)(?:\|[^\[\]]*)?\]\])"_s);
    static const QRegularExpression markdown(
        uR"((!?)\[(?:[^\[\]]|\[[^\[\]]*\])*\]\(\s*(<[^>]*>|[^\s)]+)(?:\s+(?:"[^"]*"|'[^']*'))?\s*\))"_s);
    static const QRegularExpression tag(
        uR"((?:^|(?<=[\s(,]))#([\p{L}\p{N}_/-]*[\p{L}_/-][\p{L}\p{N}_/-]*))"_s);
    static const QRegularExpression task(uR"(^\s*(?:[-*+]|\d+[.)])\s+\[([ xX])\]\s+(.*)$)"_s);
    static const QRegularExpression fence(uR"(^\s{0,3}(```+|~~~+))"_s);

    NoteScan scan;
    QString openFence;
    int number = firstLine - 1;
    for (const QStringView raw : body.tokenize(u'\n')) {
        ++number;
        QString line = raw.toString();
        if (line.endsWith(u'\r'))
            line.chop(1);

        const QRegularExpressionMatch fenceMatch = fence.match(line);
        if (!openFence.isEmpty()) {
            if (fenceMatch.hasMatch() && fenceMatch.captured(1).front() == openFence.front() &&
                fenceMatch.captured(1).size() >= openFence.size())
                openFence.clear();
            continue;
        }
        if (fenceMatch.hasMatch()) {
            openFence = fenceMatch.captured(1);
            continue;
        }
        // Indented code, four spaces or a tab in, unless it is part of a list.
        if ((line.startsWith(u"    "_s) || line.startsWith(u'\t')) &&
            !QStringView(line).trimmed().startsWith(u'-') &&
            !QStringView(line).trimmed().startsWith(u'*'))
            continue;

        if (const QRegularExpressionMatch m = task.match(line); m.hasMatch())
            scan.tasks.append({number, m.captured(1) != u" ", m.captured(2).trimmed()});

        const QString text = withoutCodeSpans(line);
        QString rest = text;
        // Each with where it starts, so the line's links keep the order they are written in.
        QList<std::pair<qsizetype, NoteScan::Link>> found;
        for (const QRegularExpressionMatch &m : wiki.globalMatch(text)) {
            found.append({m.capturedStart(),
                          {m.captured(2).trimmed(),
                           m.captured(1).isEmpty() ? u"wiki"_s : u"embed"_s, number}});
            // Blanked so the tag search does not read "[[#Heading]]" as a tag.
            rest.replace(m.capturedStart(), m.capturedLength(), QString(m.capturedLength(), u' '));
        }
        for (const QRegularExpressionMatch &m : markdown.globalMatch(rest)) {
            QString target = m.captured(2);
            if (target.startsWith(u'<'))
                target = target.mid(1, target.size() - 2);
            if (!target.isEmpty() && !target.startsWith(u'#') && !isUrl(target))
                found.append({m.capturedStart(),
                              {target, m.captured(1).isEmpty() ? u"link"_s : u"embed"_s, number}});
            rest.replace(m.capturedStart(), m.capturedLength(), QString(m.capturedLength(), u' '));
        }
        std::ranges::sort(found, {}, &std::pair<qsizetype, NoteScan::Link>::first);
        for (const auto &[at, link] : std::as_const(found))
            scan.links.append(link);
        for (const QRegularExpressionMatch &m : tag.globalMatch(rest)) {
            QString name = m.captured(1);
            while (name.endsWith(u'/'))
                name.chop(1);
            if (!name.isEmpty() && !scan.tags.contains(name, Qt::CaseInsensitive))
                scan.tags.append(name);
        }
    }
    return scan;
}

int bodyFirstLine(QStringView note, QStringView body)
{
    const qsizetype offset = body.data() - note.data();
    if (offset <= 0 || offset > note.size())
        return 1;
    return int(note.first(offset).count(u'\n')) + 1;
}

} // namespace jot
