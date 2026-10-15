#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QVariantMap>

namespace jot {

class MarkdownHighlighter;

/// Puts Markdown styling on a TextEdit's document, in the theme's colors and type, and
/// says what a click lands on. Set `document` to the TextEdit's textDocument and keep
/// `cursorPosition` bound to its cursor.
class MarkdownStyler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(
        int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    /// The editor's body text size, which headings are scaled from.
    Q_PROPERTY(int pixelSize READ pixelSize WRITE setPixelSize NOTIFY pixelSizeChanged)

public:
    explicit MarkdownStyler(QObject *parent = nullptr);
    ~MarkdownStyler() override;

    [[nodiscard]] QQuickTextDocument *document() const { return m_document; }
    void setDocument(QQuickTextDocument *document);
    [[nodiscard]] int cursorPosition() const { return m_cursorPosition; }
    void setCursorPosition(int position);
    [[nodiscard]] int pixelSize() const { return m_pixelSize; }
    void setPixelSize(int size);

    /// What is at `position`: `{kind, target, position}`, where kind is "", "link",
    /// "wikilink", "url" or "task"; for a task, `position` is where its mark is.
    Q_INVOKABLE QVariantMap hitAt(int position) const;

Q_SIGNALS:
    void documentChanged();
    void cursorPositionChanged();
    void pixelSizeChanged();

private:
    void applyLook();

    QPointer<QQuickTextDocument> m_document;
    MarkdownHighlighter *m_highlighter = nullptr;
    int m_cursorPosition = 0;
    int m_pixelSize = 16;
};

} // namespace jot
