#pragma once

#include "jot/MonthModel.h"

#include <QQmlEngine>

namespace jot {

/// The month the date picker shows, made in QML.
struct MonthModelForeign
{
    Q_GADGET
    QML_FOREIGN(jot::MonthModel)
    QML_NAMED_ELEMENT(MonthModel)
};

} // namespace jot
