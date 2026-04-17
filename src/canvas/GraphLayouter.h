#pragma once

#include <QHash>
#include <QPair>
#include <QPointF>
#include <QString>
#include <QVector>

namespace GraphLayouter
{
int layoutColumn(const QString &mediaClass);
QPointF gridPosition(int column, int row);

// Input order is preserved when assigning rows inside each column.
QHash<quint32, QPointF> computeGridPositions(const QVector<QPair<quint32, QString>> &nodes);
} // namespace GraphLayouter
