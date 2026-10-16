#pragma once

#include "jot/CodeColors.h"

#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/State>

#include <QList>
#include <QString>
#include <QTextBlockUserData>
#include <QTextCharFormat>

namespace jot {

/// Colors the lines of fenced code blocks by their language, through KSyntaxHighlighting,
/// one line at a time so the Markdown highlighter can carry the parser along its blocks.
/// The colors are the app theme's, not KSyntaxHighlighting's own themes.
class CodeHighlighter : public KSyntaxHighlighting::AbstractHighlighter
{
public:
    /// A format for part of a line.
    struct Span
    {
        int start;
        int length;
        QTextCharFormat format;
    };

    /// What a code block's line keeps on its text block: the block's language, as its
    /// fence names it, and where the parser was at the end of the line.
    struct Line : QTextBlockUserData
    {
        QString language;
        KSyntaxHighlighting::State state;
    };

    CodeHighlighter();

    /// The colors for each kind of word, and the weight keywords are drawn in.
    void setColors(const CodeColors &colors, int boldWeight);

    /// The formats for `text`, a line of code in `language`, starting from `state`, which
    /// becomes the state after the line. Nothing for a language it does not know.
    QList<Span> highlight(QStringView text, const QString &language,
                          KSyntaxHighlighting::State &state);

protected:
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

private:
    QList<Span> m_spans;
    QString m_language;
    CodeColors m_colors;
    int m_boldWeight = 700;
};

} // namespace jot
