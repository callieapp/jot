#pragma once

#include "jot/Settings.h"

#include <QCoreApplication>
#include <QQmlEngine>

namespace jot {

/// The user's preferences as a QML singleton. main.cpp picks the instance;
/// without one, such as in the gallery, it is the user's own settings file.
struct SettingsForeign
{
    Q_GADGET
    QML_FOREIGN(jot::Settings)
    QML_NAMED_ELEMENT(Settings)
    QML_SINGLETON

public:
    static jot::Settings *create(QQmlEngine *, QJSEngine *)
    {
        if (!s_instance)
            s_instance =
                new jot::Settings(jot::Settings::defaultPath(), QCoreApplication::instance());
        QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
        return s_instance;
    }

    static inline jot::Settings *s_instance = nullptr;
};

} // namespace jot
