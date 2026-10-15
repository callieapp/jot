#pragma once

#include <QObject>
#include <QQmlEngine>

namespace jot {

/// The title bar's help menu: debug info, the logs folder and bug reports.
class SupportActions : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Support)
    QML_SINGLETON

public:
    using QObject::QObject;

    /// Puts the `jot doctor` report on the clipboard, then emits copied().
    Q_INVOKABLE void copyDebugInfo();
    Q_INVOKABLE void openLogs();
    /// Opens the vault's folder ("vault"), or the folder holding the settings ("config")
    /// or the search index ("cache").
    Q_INVOKABLE void openFolder(const QString &which);
    /// Puts `text` on the clipboard, then emits copied().
    Q_INVOKABLE void copyText(const QString &text);
    /// Opens a new GitHub issue with the report filled in.
    Q_INVOKABLE void reportBug();

Q_SIGNALS:
    void copied();

private:
    [[nodiscard]] static QString collect();
};

} // namespace jot
