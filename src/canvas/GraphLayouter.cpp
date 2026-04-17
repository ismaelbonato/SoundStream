#include "GraphLayouter.h"

namespace
{
constexpr qreal kColX[] = { 60.0, 460.0, 860.0 };
constexpr qreal kRowStep = 120.0;
constexpr qreal kTop = 60.0;
}

namespace GraphLayouter
{

int layoutColumn(const QString &mediaClass)
{
    if (mediaClass.contains(QLatin1String("Sink"), Qt::CaseInsensitive)) return 2;
    if (mediaClass.contains(QLatin1String("Source"), Qt::CaseInsensitive)) return 0;
    return 1; // Duplex, Filter, Stream, unknown → centre
}

QPointF gridPosition(int column, int row)
{
    return { kColX[column], kTop + row * kRowStep };
}

QHash<quint32, QPointF> computeGridPositions(const QVector<QPair<quint32, QString>> &nodes)
{
    QHash<quint32, QPointF> positions;
    int rowCount[3] = { 0, 0, 0 };

    for (const auto &[id, mediaClass] : nodes) {
        const int column = layoutColumn(mediaClass);
        positions.insert(id, gridPosition(column, rowCount[column]));
        ++rowCount[column];
    }

    return positions;
}

} // namespace GraphLayouter
