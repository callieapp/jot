#pragma once

#include <QColor>
#include <QHash>
#include <QSize>
#include <QSyntaxHighlighter>
#include <QTextBlockUserData>
#include <QTextCharFormat>

#include <functional>

namespace jot {

/// Styles Markdown as it is typed, in the manner of a live preview, without changing a
/// character: headings grow, emphasis shows, and the marks that make them fade, folding
/// away entirely except on the line the cursor is on. Formats are not part of the text or
/// its undo history, so the note stays exactly what the user wrote.
class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /// What the styles are made of, from the theme.
    struct Look
    {
        QColor text;
        QColor muted;
        QColor faint;
        QColor accent;
        QColor codeBackground;
        QString family;
        QString monoFamily;
        QString displayFamily;
        int pixelSize = 15;
        int strongWeight = 600;
        int boldWeight = 700;
        /// The widest and tallest an image is shown inline.
        int imageWidth = 600;
        int imageHeight = 360;
    };

    /// An image shown in place of a line that holds only its link.
    struct Image : QTextBlockUserData
    {
        QString path;
        QSize size;
    };

    /// Finds the file an image link reaches: its target as written, and whether it is a
    /// wikilink. Empty when it reaches none.
    using Resolver = std::function<QString(const QString &target, bool wiki)>;

    /// What sits under a position: a link's target, or the box of a task.
    struct Hit
    {
        enum Kind { None, Link, Wikilink, Url, TaskBox };
        Kind kind = None;
        /// The link's target as written, without its label.
        QString target;
        /// For a task box, where its mark (space or x) is in the document.
        int position = -1;
    };

    explicit MarkdownHighlighter(QTextDocument *document);

    void setLook(const Look &look);
    void setResolver(Resolver resolver);
    /// The block the cursor is in shows its marks; others fold them away.
    void setCursorPosition(int position);

    [[nodiscard]] Hit hitAt(int position) const;

    /// Block states, kept so code fences and front matter carry from line to line.
    enum State { Normal = 0, InFence = 1, InFrontMatter = 2 };

protected:
    void highlightBlock(const QString &text) override;

private:
    void merge(int start, int length, const QTextCharFormat &format);
    void mark(int start, int length);
    void inlineStyles(const QString &text, int from);
    /// Shows the image a line links to, if the line is only that link; says whether it did.
    bool imageLine(const QString &text);
    [[nodiscard]] QSize imageSize(const QString &path);

    Look m_look;
    int m_cursorBlock = -1;
    bool m_cursorHere = false;
    Resolver m_resolver;
    QHash<QString, QSize> m_sizes;
    /// Where image links lead, worked out once each: a lookup can ask the whole vault.
    QHash<QString, QString> m_resolved;
};

} // namespace jot
