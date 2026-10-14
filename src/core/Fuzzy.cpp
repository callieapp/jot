#include "jot/Fuzzy.h"

#include <QList>

#include <limits>

namespace jot {

namespace {

constexpr int kMatch = 16;
constexpr int kWordStart = 24;
constexpr int kInRun = 20;
constexpr int kGap = -2;
constexpr int kNone = std::numeric_limits<int>::min() / 2;

bool startsWord(QStringView text, qsizetype i)
{
    if (i == 0)
        return true;
    const QChar before = text[i - 1];
    const QChar here = text[i];
    return !before.isLetterOrNumber() || (before.isLower() && here.isUpper()) ||
           (!before.isDigit() && here.isDigit());
}

} // namespace

std::optional<int> fuzzyScore(QStringView pattern, QStringView text, QList<qsizetype> *positions)
{
    if (pattern.isEmpty())
        return 0;
    const qsizetype n = pattern.size();
    const qsizetype m = text.size();
    if (n > m)
        return std::nullopt;

    // best[i][j]: the best score with pattern[0..i] matched and pattern[i] at text[j].
    QList<QList<int>> best(n, QList<int>(m, kNone));
    QList<QList<qsizetype>> from(n, QList<qsizetype>(m, -1));
    for (qsizetype i = 0; i < n; ++i) {
        const QChar wanted = pattern[i].toCaseFolded();
        // The best way to have matched pattern[i - 1] somewhere before j, gaps counted.
        int bestBefore = kNone;
        qsizetype bestBeforeAt = -1;
        for (qsizetype j = 0; j < m; ++j) {
            if (i > 0 && j > 0) {
                if (bestBefore != kNone)
                    bestBefore += kGap;
                if (best[i - 1][j - 1] != kNone && best[i - 1][j - 1] + kGap > bestBefore) {
                    bestBefore = best[i - 1][j - 1] + kGap;
                    bestBeforeAt = j - 1;
                }
            }
            if (text[j].toCaseFolded() != wanted)
                continue;
            const int bonus = kMatch + (startsWord(text, j) ? kWordStart : 0);
            if (i == 0) {
                // Leading characters skipped cost a little, so earlier matches win ties.
                best[i][j] = bonus - int(std::min<qsizetype>(j, 10));
                continue;
            }
            int score = kNone;
            if (j > 0 && best[i - 1][j - 1] != kNone) {
                score = best[i - 1][j - 1] + bonus + kInRun;
                from[i][j] = j - 1;
            }
            if (bestBefore != kNone && bestBefore + bonus > score) {
                score = bestBefore + bonus;
                from[i][j] = bestBeforeAt;
            }
            best[i][j] = score;
        }
    }

    qsizetype end = -1;
    int score = kNone;
    for (qsizetype j = 0; j < m; ++j) {
        if (best[n - 1][j] > score) {
            score = best[n - 1][j];
            end = j;
        }
    }
    if (end < 0)
        return std::nullopt;
    if (positions) {
        positions->resize(n);
        for (qsizetype i = n - 1, j = end; i >= 0; j = from[i][j], --i)
            (*positions)[i] = j;
    }
    // Shorter texts win between otherwise equal matches.
    return score - int(std::min<qsizetype>(m - n, 50)) / 5;
}

} // namespace jot
