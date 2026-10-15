#include "CodeHighlighter.h"

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include <QFont>

using namespace Qt::StringLiterals;

namespace jot {

namespace {

/// Loading every syntax definition takes a while, so the app shares one repository.
KSyntaxHighlighting::Repository &repository()
{
    static KSyntaxHighlighting::Repository repository;
    return repository;
}

KSyntaxHighlighting::Definition definitionFor(const QString &language)
{
    if (language.isEmpty())
        return {};
    // By name, in any case ("Python", "python"), then as a file's extension ("py", "rs").
    KSyntaxHighlighting::Definition definition = repository().definitionForName(language);
    if (definition.isValid())
        return definition;
    for (const KSyntaxHighlighting::Definition &each : repository().definitions()) {
        if (each.name().compare(language, Qt::CaseInsensitive) == 0)
            return each;
    }
    return repository().definitionForFileName(u"code."_s + language);
}

} // namespace

CodeHighlighter::CodeHighlighter()
{
    setTheme(repository().defaultTheme(KSyntaxHighlighting::Repository::DarkTheme));
}

void CodeHighlighter::setDark(bool dark)
{
    if (m_dark == dark)
        return;
    m_dark = dark;
    setTheme(repository().defaultTheme(dark ? KSyntaxHighlighting::Repository::DarkTheme
                                            : KSyntaxHighlighting::Repository::LightTheme));
}

QList<CodeHighlighter::Span> CodeHighlighter::highlight(QStringView text, const QString &language,
                                                        KSyntaxHighlighting::State &state)
{
    if (language != m_language) {
        m_language = language;
        setDefinition(definitionFor(language));
    }
    m_spans.clear();
    if (definition().isValid())
        state = highlightLine(text, state);
    return m_spans;
}

void CodeHighlighter::applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format)
{
    if (length <= 0 || format.isDefaultTextStyle(theme()))
        return;
    QTextCharFormat style;
    if (format.hasTextColor(theme()))
        style.setForeground(format.textColor(theme()));
    if (format.isBold(theme()))
        style.setFontWeight(QFont::Bold);
    if (format.isItalic(theme()))
        style.setFontItalic(true);
    m_spans.append({offset, length, style});
}

} // namespace jot
