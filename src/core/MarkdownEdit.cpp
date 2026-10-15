#include "jot/MarkdownEdit.h"

#include "jot/FrontMatter.h"

#include <QList>
#include <QRegularExpression>

#include <algorithm>
#include <ranges>

using namespace Qt::StringLiterals;

namespace jot::markdownedit {

namespace {

qsizetype lineStartOf(const QString &text, qsizetype position)
{
    return position <= 0 ? 0 : text.lastIndexOf(u'\n', position - 1) + 1;
}

qsizetype lineEndOf(const QString &text, qsizetype position)
{
    const qsizetype end = text.indexOf(u'\n', position);
    return end < 0 ? text.size() : end;
}

bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || c == u'_' || c == u'\'';
}

qsizetype runBefore(const QString &text, qsizetype position, QChar c, qsizetype limit)
{
    qsizetype n = 0;
    while (position - n - 1 >= limit && text.at(position - n - 1) == c)
        ++n;
    return n;
}

qsizetype runAfter(const QString &text, qsizetype position, QChar c, qsizetype limit)
{
    qsizetype n = 0;
    while (position + n < limit && text.at(position + n) == c)
        ++n;
    return n;
}

/// Whether runs of `n` asterisks on both sides already make `marker`: odd runs are
/// italic and runs of two or more are bold, so `***` is both.
bool starsMake(qsizetype n, const QString &marker)
{
    return marker.size() == 1 ? n % 2 == 1 : n >= 2;
}

/// Where `marker` already is: around the text from `s` to `e`, inside its ends, or neither.
enum class Placement { None, Around, Inside };

Placement placement(const QString &text, qsizetype s, qsizetype e, const QString &marker)
{
    const qsizetype m = marker.size();
    if (marker.startsWith(u'*')) {
        const qsizetype around =
            std::min(runBefore(text, s, u'*', 0), runAfter(text, e, u'*', text.size()));
        if (around > 0 && starsMake(around, marker))
            return Placement::Around;
        const qsizetype middle = s + (e - s) / 2;
        const qsizetype inside =
            std::min(runAfter(text, s, u'*', middle), runBefore(text, e, u'*', middle));
        if (inside > 0 && starsMake(inside, marker) && e - s > 2 * inside)
            return Placement::Inside;
        return Placement::None;
    }
    if (s >= m && text.mid(s - m, m) == marker && text.mid(e, m) == marker)
        return Placement::Around;
    if (e - s > 2 * m && text.mid(s, m) == marker && text.mid(e - m, m) == marker)
        return Placement::Inside;
    return Placement::None;
}

struct Wrapped
{
    QString text;
    qsizetype start;
    qsizetype end;
};

/// Adds or removes `marker` at the text from `s` to `e`, giving where that text is after.
Wrapped toggle(QString text, qsizetype s, qsizetype e, const QString &marker, bool add)
{
    const qsizetype m = marker.size();
    const Placement at = placement(text, s, e, marker);
    if (!add && at == Placement::Around) {
        text.remove(e, m);
        text.remove(s - m, m);
        return {text, s - m, e - m};
    }
    if (!add && at == Placement::Inside) {
        text.remove(e - m, m);
        text.remove(s, m);
        return {text, s, e - 2 * m};
    }
    if (add && at == Placement::None) {
        text.insert(e, marker);
        text.insert(s, marker);
        return {text, s + m, e + m};
    }
    return {text, s, e};
}

/// The part of a selection on one line, without the spaces at its ends.
struct Segment
{
    qsizetype start;
    qsizetype end;
};

Edit wrapLines(const QString &text, qsizetype from, qsizetype to, const QString &marker)
{
    QList<Segment> segments;
    for (qsizetype start = from; start < to;) {
        const qsizetype end = std::min(lineEndOf(text, start), to);
        qsizetype s = start, e = end;
        while (s < e && text.at(s).isSpace())
            ++s;
        while (e > s && text.at(e - 1).isSpace())
            --e;
        if (s < e)
            segments.append({s, e});
        start = end + 1;
    }
    const bool all = std::ranges::all_of(segments, [&](const Segment &segment) {
        return placement(text, segment.start, segment.end, marker) != Placement::None;
    });
    QString changed = text;
    // From the end, so the segments before keep their places.
    for (const Segment &segment : std::views::reverse(segments))
        changed = toggle(changed, segment.start, segment.end, marker, !all).text;
    return difference(text, changed, from, to + changed.size() - text.size());
}

/// A line split into the parts a line style changes.
struct Line
{
    QString indent;
    QString quote;
    /// A list marker or heading marks, with the space after.
    QString block;
    QString content;

    enum Kind { Plain, Bullet, Numbered, Task, Heading };
    Kind kind = Plain;
    int level = 0;

    [[nodiscard]] bool blank() const { return block.isEmpty() && content.trimmed().isEmpty(); }
    [[nodiscard]] qsizetype prefix() const { return indent.size() + quote.size() + block.size(); }
    [[nodiscard]] QString joined() const { return indent + quote + block + content; }
};

Line parse(const QString &text)
{
    static const QRegularExpression parts(
        uR"(^([ \t]*)(> ?)?((#{1,6})(?: |$)|[-*+] \[[ xX]\] |[-*+] |\d+[.)] )?(.*)$)"_s);
    const QRegularExpressionMatch match = parts.match(text);
    Line line{match.captured(1), match.captured(2), match.captured(3), match.captured(5)};
    if (!match.captured(4).isEmpty()) {
        line.kind = Line::Heading;
        line.level = int(match.captured(4).size());
    } else if (line.block.contains(u'[')) {
        line.kind = Line::Task;
    } else if (!line.block.isEmpty()) {
        line.kind = line.block.front().isDigit() ? Line::Numbered : Line::Bullet;
    }
    return line;
}

/// How far a nested item is indented: under the text of the item above it.
qsizetype nestUnit(const Line &line)
{
    return line.kind == Line::Numbered ? line.block.size() : 2;
}

bool has(const Line &line, LineStyle style)
{
    switch (style) {
    case LineStyle::Bullet: return line.kind == Line::Bullet;
    case LineStyle::Numbered: return line.kind == Line::Numbered;
    case LineStyle::Task: return line.kind == Line::Task;
    case LineStyle::Quote: return !line.quote.isEmpty();
    case LineStyle::Heading1: return line.kind == Line::Heading && line.level == 1;
    case LineStyle::Heading2: return line.kind == Line::Heading && line.level == 2;
    case LineStyle::Heading3: return line.kind == Line::Heading && line.level == 3;
    }
    return false;
}

void apply(Line &line, LineStyle style, bool remove, int number)
{
    if (style == LineStyle::Quote) {
        line.quote = remove ? QString() : u"> "_s;
        return;
    }
    if (remove) {
        line.block.clear();
        line.kind = Line::Plain;
        return;
    }
    switch (style) {
    case LineStyle::Bullet: line.block = u"- "_s; break;
    case LineStyle::Numbered: line.block = QString::number(number) + u". "_s; break;
    case LineStyle::Task: line.block = u"- [ ] "_s; break;
    case LineStyle::Heading1:
    case LineStyle::Heading2:
    case LineStyle::Heading3:
        line.block = QString(int(style) - int(LineStyle::Heading1) + 1, u'#') + u' ';
        break;
    case LineStyle::Quote: break;
    }
}

bool isFence(const QString &text, qsizetype start)
{
    return text.mid(start, lineEndOf(text, start) - start).trimmed().startsWith(u"```"_s);
}

/// Where the code block that the line at `start` sits inside opens, or -1. Fences pair up
/// from the top of the note, so it depends on how many come before the line, not on its
/// neighbours alone.
qsizetype openFenceBefore(const QString &text, qsizetype start)
{
    qsizetype open = -1;
    for (qsizetype line = 0; line < start; line = lineEndOf(text, line) + 1) {
        if (isFence(text, line))
            open = open < 0 ? line : -1;
    }
    return open;
}

/// Whether the line at `start` is code: a fence, or inside a fenced block.
bool inCode(const QString &text, qsizetype start)
{
    return isFence(text, start) || openFenceBefore(text, start) >= 0;
}

} // namespace

Edit difference(const QString &before, const QString &after, qsizetype selectionStart,
                qsizetype selectionEnd)
{
    const qsizetype shortest = std::min(before.size(), after.size());
    qsizetype prefix = 0;
    while (prefix < shortest && before.at(prefix) == after.at(prefix))
        ++prefix;
    qsizetype suffix = 0;
    while (suffix < shortest - prefix &&
           before.at(before.size() - suffix - 1) == after.at(after.size() - suffix - 1))
        ++suffix;
    return {prefix, before.size() - suffix, after.mid(prefix, after.size() - suffix - prefix),
            selectionStart, selectionEnd};
}

Edit wrap(const QString &text, qsizetype from, qsizetype to, const QString &marker)
{
    from = std::clamp<qsizetype>(from, 0, text.size());
    to = std::clamp<qsizetype>(to, 0, text.size());
    if (from > to)
        std::swap(from, to);
    const qsizetype m = marker.size();
    if (text.mid(from, to - from).contains(u'\n'))
        return wrapLines(text, from, to, marker);

    if (from == to) {
        qsizetype s = from, e = to;
        while (s > 0 && isWordChar(text.at(s - 1)))
            --s;
        while (e < text.size() && isWordChar(text.at(e)))
            ++e;
        if (s == e) {
            // A pair with nothing in it is taken away again; otherwise one is made to type in.
            if (from >= m && text.mid(from - m, m) == marker && text.mid(from, m) == marker) {
                QString changed = text;
                changed.remove(from - m, 2 * m);
                return difference(text, changed, from - m, from - m);
            }
            return difference(text, text.left(from) + marker + marker + text.mid(from), from + m,
                              from + m);
        }
        const bool add = placement(text, s, e, marker) == Placement::None;
        const Wrapped done = toggle(text, s, e, marker, add);
        const qsizetype cursor = done.start + (from - s);
        return difference(text, done.text, cursor, cursor);
    }

    qsizetype s = from, e = to;
    while (s < e && text.at(s).isSpace())
        ++s;
    while (e > s && text.at(e - 1).isSpace())
        --e;
    if (s == e)
        return difference(text, text, from, to);
    const bool add = placement(text, s, e, marker) == Placement::None;
    const Wrapped done = toggle(text, s, e, marker, add);
    return difference(text, done.text, done.start, done.end);
}

Edit styleLines(const QString &text, qsizetype from, qsizetype to, LineStyle style)
{
    from = std::clamp<qsizetype>(from, 0, text.size());
    to = std::clamp<qsizetype>(to, 0, text.size());
    if (from > to)
        std::swap(from, to);
    // A selection that ends at the start of a line leaves that line alone.
    const qsizetype last = to > from && to == lineStartOf(text, to) ? to - 1 : to;
    const qsizetype first = lineStartOf(text, from);
    const qsizetype end = lineEndOf(text, last);

    const QStringList raw = text.mid(first, end - first).split(u'\n');
    QList<Line> lines;
    for (const QString &line : raw)
        lines.append(parse(line));
    const bool allBlank = std::ranges::all_of(lines, &Line::blank);
    const auto touched = [allBlank](const Line &line) { return allBlank || !line.blank(); };
    const bool remove = std::ranges::all_of(
        lines, [&](const Line &line) { return !touched(line) || has(line, style); });

    QStringList out;
    QList<qsizetype> oldStarts, newStarts, oldPrefixes, newPrefixes, indents;
    qsizetype oldAt = first, newAt = first;
    int number = 1;
    for (Line line : std::as_const(lines)) {
        oldStarts.append(oldAt);
        newStarts.append(newAt);
        oldPrefixes.append(line.prefix());
        indents.append(line.indent.size());
        oldAt += line.joined().size() + 1;
        if (touched(line))
            apply(line, style, remove, number++);
        newPrefixes.append(line.prefix());
        out.append(line.joined());
        newAt += out.last().size() + 1;
    }
    const QString changed = text.left(first) + out.join(u'\n') + text.mid(end);

    const auto map = [&](qsizetype position) {
        if (position < first)
            return position;
        if (position > end)
            return position + changed.size() - text.size();
        qsizetype i = oldStarts.size() - 1;
        while (i > 0 && oldStarts[i] > position)
            --i;
        const qsizetype offset = position - oldStarts[i];
        if (offset < indents[i])
            return newStarts[i] + offset;
        return newStarts[i] + newPrefixes[i] + std::max<qsizetype>(0, offset - oldPrefixes[i]);
    };
    // A selection from the start of its first line keeps the marks it gains.
    const qsizetype start = from < to && from == first ? from : map(from);
    return difference(text, changed, start, map(to));
}

Edit link(const QString &text, qsizetype from, qsizetype to)
{
    from = std::clamp<qsizetype>(from, 0, text.size());
    to = std::clamp<qsizetype>(to, 0, text.size());
    if (from > to)
        std::swap(from, to);
    static const QRegularExpression url(uR"(^(?:[a-z][a-z0-9+.-]*://|mailto:)\S+$)"_s,
                                        QRegularExpression::CaseInsensitiveOption);
    const QString selected = text.mid(from, to - from);
    const bool isUrl = url.match(selected).hasMatch();
    const QString made = isUrl ? u"[]("_s + selected + u')' : u'[' + selected + u"]()"_s;
    // The cursor goes where the half still missing is typed.
    const qsizetype cursor = isUrl || selected.isEmpty() ? from + 1 : from + selected.size() + 3;
    return difference(text, text.left(from) + made + text.mid(to), cursor, cursor);
}

Edit codeBlock(const QString &text, qsizetype from, qsizetype to)
{
    from = std::clamp<qsizetype>(from, 0, text.size());
    to = std::clamp<qsizetype>(to, 0, text.size());
    if (from > to)
        std::swap(from, to);
    const qsizetype last = to > from && to == lineStartOf(text, to) ? to - 1 : to;
    const qsizetype first = lineStartOf(text, from);
    const qsizetype end = lineEndOf(text, last);
    const auto fence = [&](qsizetype start) { return isFence(text, start); };
    qsizetype open = openFenceBefore(text, first);
    if (open < 0 && fence(first))
        open = first;
    if (open >= 0) {
        qsizetype close = -1;
        for (qsizetype line = open == first ? lineEndOf(text, first) + 1 : first;
             line < text.size(); line = lineEndOf(text, line) + 1) {
            if (fence(line)) {
                close = line;
                break;
            }
        }
        const qsizetype opening = std::min(lineEndOf(text, open) + 1, text.size()) - open;
        QString changed = text;
        // The closing fence goes with the line break before it; an unclosed block loses only
        // its opening.
        if (close > 0)
            changed.remove(close - 1, lineEndOf(text, close) - close + 1);
        changed.remove(open, opening);
        const auto map = [&](qsizetype position) {
            return std::max(open, position >= open + opening ? position - opening : open);
        };
        return difference(text, changed, std::min(map(from), qsizetype(changed.size())),
                          std::min(map(to), qsizetype(changed.size())));
    }
    const QString changed =
        text.left(first) + u"```\n"_s + text.mid(first, end - first) + u"\n```"_s + text.mid(end);
    return difference(text, changed, from + 4, to + 4);
}

std::optional<Edit> continueList(const QString &text, qsizetype from, qsizetype to)
{
    from = std::clamp<qsizetype>(from, 0, text.size());
    to = std::clamp<qsizetype>(to, 0, text.size());
    if (from > to)
        std::swap(from, to);
    const qsizetype start = lineStartOf(text, from);
    // A selection over several lines is replaced as a plain line break would replace it.
    if (to > lineEndOf(text, from))
        return std::nullopt;
    if (inCode(text, start))
        return std::nullopt;
    const Line line = parse(text.mid(start, lineEndOf(text, from) - start));
    const bool listed =
        line.kind == Line::Bullet || line.kind == Line::Numbered || line.kind == Line::Task;
    if ((!listed && line.quote.isEmpty()) || from - start < line.prefix())
        return std::nullopt;

    if (line.content.trimmed().isEmpty()) {
        // An empty item ends the list, or steps out of a nested one first.
        Line ended = line;
        if (!ended.indent.isEmpty() && listed) {
            // As far as Tab nested it, so one Return undoes one Tab.
            const qsizetype unit = ended.indent.startsWith(u'\t') ? 1 : nestUnit(ended);
            ended.indent.remove(0, std::min(unit, ended.indent.size()));
        } else if (listed) {
            ended.block.clear();
        } else {
            ended.quote.clear();
        }
        const qsizetype end = lineEndOf(text, from);
        const QString changed = text.left(start) + ended.joined() + text.mid(end);
        const qsizetype cursor = start + ended.prefix();
        return difference(text, changed, cursor, cursor);
    }

    // A quoted heading carries on as quoted text, not as another heading.
    QString block = listed ? line.block : QString();
    if (line.kind == Line::Task) {
        block = block.left(2) + u"[ ] "_s;
    } else if (line.kind == Line::Numbered) {
        const qsizetype digits = block.indexOf(QRegularExpression(u"[.)]"_s));
        block = QString::number(block.left(digits).toInt() + 1) + block.mid(digits);
    }
    const QString next = u'\n' + line.indent + line.quote + block;
    const QString changed = text.left(from) + next + text.mid(to);
    const qsizetype cursor = from + next.size();
    return difference(text, changed, cursor, cursor);
}

std::optional<Edit> indentList(const QString &text, qsizetype from, qsizetype to, bool outdent)
{
    from = std::clamp<qsizetype>(from, 0, text.size());
    to = std::clamp<qsizetype>(to, 0, text.size());
    if (from > to)
        std::swap(from, to);
    const qsizetype last = to > from && to == lineStartOf(text, to) ? to - 1 : to;
    const qsizetype first = lineStartOf(text, from);
    const qsizetype end = lineEndOf(text, last);

    QStringList out;
    // How far each line's start moved, to carry the selection along.
    QList<qsizetype> starts, shifts;
    qsizetype at = first;
    if (inCode(text, first))
        return std::nullopt;
    for (const QString &raw : text.mid(first, end - first).split(u'\n')) {
        Line line = parse(raw);
        // Nesting inside a quote would need its indent after the marker, which the line
        // parser does not keep, so quoted items are left to Tab's usual behaviour.
        if ((line.kind != Line::Bullet && line.kind != Line::Numbered && line.kind != Line::Task) ||
            !line.quote.isEmpty())
            return std::nullopt;
        const qsizetype unit = nestUnit(line);
        const qsizetype before = line.indent.size();
        if (!outdent) {
            line.indent += QString(unit, u' ');
        } else if (line.indent.startsWith(u'\t')) {
            line.indent.remove(0, 1);
        } else {
            qsizetype spaces = 0;
            while (spaces < std::min(unit, line.indent.size()) && line.indent.at(spaces) == u' ')
                ++spaces;
            line.indent.remove(0, spaces);
        }
        starts.append(at);
        shifts.append(line.indent.size() - before);
        at += raw.size() + 1;
        out.append(line.joined());
    }
    const QString changed = text.left(first) + out.join(u'\n') + text.mid(end);
    const auto map = [&](qsizetype position) {
        qsizetype i = starts.size() - 1;
        while (i > 0 && starts[i] > position)
            --i;
        qsizetype moved = 0;
        for (qsizetype j = 0; j < i; ++j)
            moved += shifts[j];
        // Indenting carries the whole line along; outdenting pulls a position inside the
        // removed indent back to the line's start.
        const qsizetype shift = shifts[i];
        return position + moved + (shift >= 0 ? shift : std::max(shift, starts[i] - position));
    };
    // A selection from the start of its first line keeps the indent it gains.
    const qsizetype start = from < to && from == first ? from : map(from);
    return difference(text, changed, start, map(to));
}

Edit setKey(const QString &text, qsizetype from, qsizetype to, const QString &key,
            const QVariant &value)
{
    const QString changed = setFrontMatterKey(text, key, value);
    const Edit edit = difference(text, changed, 0, 0);
    const qsizetype delta = changed.size() - text.size();
    const auto map = [&](qsizetype position) {
        if (position <= edit.start)
            return position;
        if (position >= edit.end)
            return position + delta;
        return edit.start + edit.text.size();
    };
    return difference(text, changed, map(from), map(to));
}

} // namespace jot::markdownedit
