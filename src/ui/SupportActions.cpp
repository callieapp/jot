#include "SupportActions.h"

#include "VaultController.h"

#include "jot/Diagnostics.h"
#include "jot/LogFile.h"
#include "jot/Settings.h"
#include "jot/UpdateCheck.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace jot {

QString SupportActions::collect()
{
    return diagnostics::collect(
        QStringLiteral("Qt platform: %1").arg(QGuiApplication::platformName()));
}

void SupportActions::copyDebugInfo()
{
    QGuiApplication::clipboard()->setText(collect());
    Q_EMIT copied();
}

void SupportActions::openLogs()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(logfile::defaultDirectory()));
}

void SupportActions::openFolder(const QString &which)
{
    if (which == u"vault"_s) {
        if (const VaultController *vault = VaultController::instance(); vault && vault->isOpen())
            QDesktopServices::openUrl(QUrl::fromLocalFile(vault->root()));
        return;
    }
    const QString file =
        which == u"config"_s ? Settings::defaultPath() : UpdateCheck::defaultPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file).absolutePath()));
}

void SupportActions::copyText(const QString &text)
{
    QGuiApplication::clipboard()->setText(text);
    Q_EMIT copied();
}

void SupportActions::reportBug()
{
    QDesktopServices::openUrl(diagnostics::issueUrl(collect()));
}

} // namespace jot
