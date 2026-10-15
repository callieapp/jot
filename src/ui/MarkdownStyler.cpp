#include "MarkdownStyler.h"

#include "MarkdownHighlighter.h"
#include "ThemeController.h"

using namespace Qt::StringLiterals;

namespace jot {

MarkdownStyler::MarkdownStyler(QObject *parent) : QObject(parent)
{
    connect(ThemeController::instance(), &ThemeController::changed, this,
            &MarkdownStyler::applyLook);
}

MarkdownStyler::~MarkdownStyler() = default;

void MarkdownStyler::setDocument(QQuickTextDocument *document)
{
    if (m_document == document)
        return;
    delete m_highlighter;
    m_highlighter = nullptr;
    m_document = document;
    if (document && document->textDocument()) {
        m_highlighter = new MarkdownHighlighter(document->textDocument());
        applyLook();
        m_highlighter->setCursorPosition(m_cursorPosition);
    }
    Q_EMIT documentChanged();
}

void MarkdownStyler::setCursorPosition(int position)
{
    if (m_cursorPosition == position)
        return;
    m_cursorPosition = position;
    if (m_highlighter)
        m_highlighter->setCursorPosition(position);
    Q_EMIT cursorPositionChanged();
}

void MarkdownStyler::setPixelSize(int size)
{
    if (m_pixelSize == size)
        return;
    m_pixelSize = size;
    applyLook();
    Q_EMIT pixelSizeChanged();
}

void MarkdownStyler::applyLook()
{
    if (!m_highlighter)
        return;
    const ThemeController &theme = *ThemeController::instance();
    MarkdownHighlighter::Look look;
    look.text = theme.text();
    look.muted = theme.textMuted();
    look.faint = theme.textFaint();
    look.accent = theme.accent();
    look.codeBackground = theme.surfaceAlt();
    look.family = theme.fontFamily();
    look.monoFamily = theme.monoFontFamily();
    look.displayFamily = theme.displayFontFamily();
    look.pixelSize = m_pixelSize;
    look.strongWeight = theme.weightStrong();
    look.boldWeight = theme.weightBold();
    m_highlighter->setLook(look);
}

QVariantMap MarkdownStyler::hitAt(int position) const
{
    if (!m_highlighter)
        return {};
    const MarkdownHighlighter::Hit hit = m_highlighter->hitAt(position);
    static const char *kinds[] = {"", "link", "wikilink", "url", "task"};
    return {{u"kind"_s, QLatin1String(kinds[hit.kind])},
            {u"target"_s, hit.target},
            {u"position"_s, hit.position}};
}

} // namespace jot
