#include "MarkdownStyler.h"

#include "MarkdownHighlighter.h"
#include "ThemeController.h"
#include "VaultController.h"

#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace jot {

MarkdownStyler::MarkdownStyler(QObject *parent) : QObject(parent), m_widthSettle(new QTimer(this))
{
    m_widthSettle->setSingleShot(true);
    m_widthSettle->setInterval(150);
    connect(m_widthSettle, &QTimer::timeout, this, &MarkdownStyler::applyLook);
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
        m_highlighter->setResolver([](const QString &target, bool wiki) {
            VaultController *vault = VaultController::instance();
            return vault ? vault->attachmentPath(target, wiki) : QString();
        });
        applyLook();
        m_highlighter->setCursorPosition(m_cursorPosition);
        connect(document->textDocument(), &QTextDocument::contentsChanged, this,
                &MarkdownStyler::scheduleImages);
    }
    Q_EMIT documentChanged();
    scheduleImages();
}

void MarkdownStyler::setCursorPosition(int position)
{
    if (m_cursorPosition == position)
        return;
    m_cursorPosition = position;
    if (m_highlighter)
        m_highlighter->setCursorPosition(position);
    Q_EMIT cursorPositionChanged();
    scheduleImages();
}

void MarkdownStyler::setPixelSize(int size)
{
    if (m_pixelSize == size)
        return;
    m_pixelSize = size;
    applyLook();
    Q_EMIT pixelSizeChanged();
}

void MarkdownStyler::setWidth(int width)
{
    if (m_width == width)
        return;
    m_width = width;
    m_widthSettle->start();
    Q_EMIT widthChanged();
}

void MarkdownStyler::scheduleImages()
{
    if (m_imagesPending)
        return;
    m_imagesPending = true;
    QTimer::singleShot(0, this, [this] {
        m_imagesPending = false;
        Q_EMIT imagesChanged();
    });
}

QVariantList MarkdownStyler::images() const
{
    QVariantList images;
    if (!m_document || !m_document->textDocument())
        return images;
    for (QTextBlock block = m_document->textDocument()->begin(); block.isValid();
         block = block.next()) {
        if (const auto *image = dynamic_cast<const MarkdownHighlighter::Image *>(block.userData()))
            images.append(QVariantMap{{u"position"_s, block.position()},
                                      {u"source"_s, QUrl::fromLocalFile(image->path)},
                                      {u"width"_s, image->size.width()},
                                      {u"height"_s, image->size.height()}});
    }
    return images;
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
    look.imageWidth = m_width;
    look.imageHeight = theme.imageHeight();
    m_highlighter->setLook(look);
    scheduleImages();
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
