#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include <functional>

namespace jot::cli {

/// One way to finish the word being typed, and what it is.
struct Candidate
{
    QString value;
    QString about;

    bool operator==(const Candidate &) const = default;
};

/// What `jot __complete` offers the shell completion scripts for the words
/// typed so far. The app supplies what each command's arguments and each
/// option's values can be; this reads the line and filters.
class Completion
{
public:
    struct Option
    {
        /// A long name, or a letter, without the dashes.
        QString name;
        QString about;
        bool takesValue = false;
    };

    /// What can follow, for the app to answer.
    struct Sources
    {
        /// The command and the arguments before the one being typed, as
        /// `{"completions"}` while typing the shell's name. Given what was typed too.
        std::function<QList<Candidate>(const QStringList &positional, const QString &typed)>
            arguments;
        /// What an option, by its long name, takes.
        std::function<QList<Candidate>(const QString &option)> values;
    };

    Completion(QList<Candidate> commands, QList<Option> options, Sources sources);

    /// `words` follow the command's own name, the last being the one typed so
    /// far, which may be empty. Gives the candidates that start with it.
    [[nodiscard]] QList<Candidate> complete(const QStringList &words) const;

    /// The words of a command line up to the cursor, as the shell reads them:
    /// quotes and backslashes taken out, and an empty last word when the line
    /// ends in a space. The first, the command's own name, is left out.
    [[nodiscard]] static QStringList wordsOf(const QString &line);

    /// Candidates without descriptions.
    [[nodiscard]] static QList<Candidate> plain(const QStringList &values);

private:
    QList<Candidate> m_commands;
    QList<Option> m_options;
    Sources m_sources;
};

} // namespace jot::cli
