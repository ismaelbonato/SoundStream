// canvas/LinkItem.cpp
#include "LinkItem.h"

#include <QApplication>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPalette>

namespace {
QColor mediaBaseColor(const QString &mediaType)
{
    if (mediaType.contains(QLatin1String("audio"), Qt::CaseInsensitive)) {
        return QColor(0x3a, 0x9a, 0xff);
    }
    if (mediaType.contains(QLatin1String("midi"), Qt::CaseInsensitive)) {
        return QColor(0xaf, 0x5d, 0xff);
    }
    if (mediaType.contains(QLatin1String("video"), Qt::CaseInsensitive)
        || mediaType.contains(QLatin1String("image"), Qt::CaseInsensitive)
        || mediaType.contains(QLatin1String("v4l2"), Qt::CaseInsensitive)
        || mediaType.contains(QLatin1String("video4linux"), Qt::CaseInsensitive)
        || mediaType.contains(QLatin1String("camera"), Qt::CaseInsensitive)) {
        return QColor(0xff, 0x9f, 0x43);
    }
    return QColor(0x7a, 0x8a, 0x9a);
}
} // namespace

LinkItem::LinkItem(const LinkData &itemData, QGraphicsItem *parent)
    : Item(parent)
    , itemData(itemData)
    , srcPt(0, 0)
    , dstPt(0, 0)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(0);
    setAcceptHoverEvents(true);

    _refreshColors();
}

void LinkItem::setMediaType(QString mediaTypeValue)
{
    if (mediaType == mediaTypeValue) {
        return;
    }
    mediaType = std::move(mediaTypeValue);
    _refreshColors();
}

void LinkItem::_refreshColors()
{
    QColor base = mediaBaseColor(mediaType);
    QColor line = base;
    if (!itemData.active) {
        line.setAlpha(160);
    }

    setForeground(line);
    setBackground(base.lighter(130));
    update();
}

// ── Geometry ───────────────────────────────────────────────────────────────

void LinkItem::setEndpoints(QPointF src, QPointF dst)
{
    srcPt = src;
    dstPt = dst;
    _rebuildPath(srcPt, dstPt);
}

void LinkItem::_rebuildPath(QPointF src, QPointF dst)
{
    prepareGeometryChange();
    qreal off = qMax(60.0, qAbs(dst.x() - src.x()) * 0.45);
    linkPath = QPainterPath();
    linkPath.moveTo(src);
    linkPath.cubicTo(src + QPointF(off, 0), dst - QPointF(off, 0), dst);
}

// ── QGraphicsItem interface ────────────────────────────────────────────────

QRectF LinkItem::boundingRect() const
{
    // Pad for pen width and hit area
    return linkPath.boundingRect().adjusted(-kHitWidth,
                                            -kHitWidth,
                                            kHitWidth,
                                            kHitWidth);
}

void LinkItem::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    const bool sel = isSelected();
    const QColor color = sel ? QApplication::palette().color(QPalette::Highlight)
                             : foreground();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(
        QPen(color, sel ? 3.0 : kLineWidth, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(linkPath);
}

QPainterPath LinkItem::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(kHitWidth);
    stroker.setCapStyle(Qt::RoundCap);
    return stroker.createStroke(linkPath);
}

// ── Interaction ────────────────────────────────────────────────────────────

QVariant LinkItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // No per-change logic needed beyond the base — selection appearance is
    // handled in paint() by checking isSelected() directly.
    return Item::itemChange(change, value);
}

void LinkItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    Q_EMIT removeRequested(itemData.id);
}

void LinkItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    setSelected(true);
    QMenu menu;
    const QAction *act = menu.addAction(QStringLiteral("Disconnect"));
    if (menu.exec(event->screenPos()) == act) {
        Q_EMIT removeRequested(itemData.id);
    }
}
