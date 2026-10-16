#include "CodeHighlighter.h"

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

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
    // Formats are only valid with a theme set, though the colors come from setColors().
    setTheme(repository().defaultTheme(KSyntaxHighlighting::Repository::DarkTheme));
}

void CodeHighlighter::setColors(const CodeColors &colors, int boldWeight)
{
    m_colors = colors;
    m_boldWeight = boldWeight;
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
    using Style = KSyntaxHighlighting::Theme::TextStyle;
    if (length <= 0)
        return;
    QTextCharFormat style;
    switch (format.textStyle()) {
    case Style::Keyword:
    case Style::ControlFlow:
        style.setForeground(m_colors.keyword);
        style.setFontWeight(m_boldWeight);
        break;
    case Style::Import:
    case Style::Preprocessor:
    case Style::Attribute: style.setForeground(m_colors.keyword); break;
    case Style::String:
    case Style::VerbatimString:
    case Style::SpecialString:
    case Style::Char:
    case Style::SpecialChar: style.setForeground(m_colors.string); break;
    case Style::DecVal:
    case Style::BaseN:
    case Style::Float:
    case Style::Constant: style.setForeground(m_colors.number); break;
    case Style::DataType:
    case Style::BuiltIn:
    case Style::Function:
    case Style::Extension: style.setForeground(m_colors.type); break;
    case Style::Comment:
    case Style::Documentation:
    case Style::Annotation:
    case Style::CommentVar:
    case Style::RegionMarker:
        style.setForeground(m_colors.comment);
        style.setFontItalic(true);
        break;
    default:
        // Variables, operators and the rest read as plain code.
        return;
    }
    m_spans.append({offset, length, style});
}

} // namespace jot
