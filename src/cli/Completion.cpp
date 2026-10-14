#include "Completion.h"

using namespace Qt::StringLiterals;

namespace jot::cli {

Completion::Completion(QList<Candidate> commands, QList<Option> options, Sources sources)
    : m_commands(std::move(commands)), m_options(std::move(options)), m_sources(std::move(sources))
{}

QStringList Completion::wordsOf(const QString &line)
{
    QStringList words;
    QString word;
    bool inWord = false;
    QChar quote;
    for (qsizetype i = 0; i < line.size(); ++i) {
        const QChar c = line.at(i);
        if (!quote.isNull()) {
            if (c == quote)
                quote = QChar();
            else if (c == u'\\' && quote == u'"' && i + 1 < line.size())
                word += line.at(++i);
            else
                word += c;
        } else if (c == u'\'' || c == u'"') {
            quote = c;
            inWord = true;
        } else if (c == u'\\') {
            if (i + 1 < line.size())
                word += line.at(++i);
            inWord = true;
        } else if (c.isSpace()) {
            if (inWord)
                words.append(std::exchange(word, QString()));
            inWord = false;
        } else {
            word += c;
            inWord = true;
        }
    }
    // The word being typed, even when nothing of it is yet.
    words.append(word);
    return words.mid(1);
}

QList<Candidate> Completion::complete(const QStringList &words) const
{
    QString typed = words.isEmpty() ? QString() : words.last();
    const QStringList before = words.mid(0, words.size() - 1);

    // --name or -n, by the dashes it is written with.
    const auto option = [this](const QString &word) -> const Option * {
        for (const Option &o : m_options) {
            if (word == (o.name.size() == 1 ? u"-"_s : u"--"_s) + o.name)
                return &o;
        }
        return nullptr;
    };

    QList<Candidate> found;
    QString prefix;
    const Option *previous = before.isEmpty() ? nullptr : option(before.last());
    if (typed.startsWith(u"--"_s) && typed.contains(u'=')) {
        // --name=value: the value, written after the name.
        prefix = typed.section(u'=', 0, 0) + u'=';
        typed = typed.mid(prefix.size());
        if (const Option *named = option(prefix.chopped(1)); named && named->takesValue)
            found = m_sources.values ? m_sources.values(named->name) : QList<Candidate>();
    } else if (previous && previous->takesValue) {
        found = m_sources.values ? m_sources.values(previous->name) : QList<Candidate>();
    } else if (typed.startsWith(u'-')) {
        for (const Option &each : m_options) {
            if (each.name.size() > 1)
                found.append({u"--"_s + each.name, each.about});
        }
    } else {
        // The words that are not options or their values.
        QStringList positional;
        for (qsizetype i = 0; i < before.size(); ++i) {
            const QString &word = before.at(i);
            if (!word.startsWith(u'-')) {
                positional.append(word);
                continue;
            }
            const Option *named = option(word);
            if (named && named->takesValue)
                ++i;
        }
        if (positional.isEmpty())
            found = m_commands;
        else if (m_sources.arguments)
            found = m_sources.arguments(positional, typed);
    }

    found.removeIf([&typed](const Candidate &c) { return !c.value.startsWith(typed); });
    for (Candidate &candidate : found)
        candidate.value.prepend(prefix);
    return found;
}

QList<Candidate> Completion::plain(const QStringList &values)
{
    QList<Candidate> result;
    for (const QString &value : values)
        result.append({value, {}});
    return result;
}

} // namespace jot::cli
