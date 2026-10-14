#include "jot/Identity.h"

using namespace Qt::StringLiterals;

namespace jot::identity {

Names namesFor(bool devel)
{
    const QString base = QStringLiteral(JOT_APP_ID);
    if (devel)
        return {base + u".Devel"_s, u"jot-devel"_s, u"Jot Devel"_s};
    return {base, u"jot"_s, u"Jot"_s};
}

QString appId()
{
    return namesFor(isDevel()).appId;
}

QString dirName()
{
    return namesFor(isDevel()).dirName;
}

QString displayName()
{
    return namesFor(isDevel()).displayName;
}

bool isDevel()
{
    return JOT_DEVEL_BUILD;
}

} // namespace jot::identity
