// canvas/PortItem.cpp
#include "PortItem.h"
#include "NodeItem.h"

#include <QApplication>
#include <QFont>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsTextItem>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyleOptionGraphicsItem>

namespace {
QColor mediaBaseColor(const QString &mediaType,
                      const QString &portName,
                      const QString &direction)
{
    const QString hint = (mediaType + QLatin1Char(' ') + portName).toLower();

    // Keep MIDI/control ports visually distinct.
    if (hint.contains(QLatin1String("midi"))
        || hint.contains(QLatin1String("control"))) {
        return QColor(0xaf, 0x5d, 0xff);
    }

    if (hint.contains(QLatin1String("video"))
        || hint.contains(QLatin1String("image"))
        || hint.contains(QLatin1String("v4l2"))
        || hint.contains(QLatin1String("video4linux"))
        || hint.contains(QLatin1String("camera"))
        || hint.contains(QLatin1String("webcam"))) {
        return QColor(0xff, 0x9f, 0x43);
    }

    const bool isOut = (direction == QLatin1String("out"));

    // Match node accent palette:
    // - input  -> Sink blue
    // - output -> Source green
    return isOut ? QColor(0x3a, 0xbf, 0x6e) : QColor(0x3a, 0x7e, 0xbf);
}
} // namespace

PortItem::PortItem(const PortData &itemData, NodeItem *parentNode)
    : Item(parentNode)
    , itemData(itemData)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setZValue(2);
    setCursor(Qt::CrossCursor);
    setToolTip(QString::fromStdString(itemData.name));

    const QColor mediaColor
        = mediaBaseColor(QString::fromStdString(itemData.mediaType),
                         QString::fromStdString(itemData.name),
                         QString::fromStdString(itemData.direction));
    setBackground(mediaColor);
    setForeground(Qt::white);

    // Text child — label lives inside the chip
    textItem = new QGraphicsTextItem(this);
    QFont font = textItem->font();
    font.setPointSizeF(font.pointSizeF() * 0.75);
    textItem->setFont(font);
    textItem->setDefaultTextColor(Qt::white);

    QString label = QString::fromStdString(itemData.name).simplified();
    textItem->setPlainText(label);

    buildShape();
}

void PortItem::buildShape()
{
    const bool isOut = (QString::fromStdString(itemData.direction)
                        == QLatin1String("out"));
    const QRectF textRect = textItem->boundingRect().adjusted(0, +2, 0, -2);
    const qreal chipW = textRect.width() + (2 * kPad);
    const qreal chipH = textRect.height();
    const qreal halfH = chipH / 2;

    // Inputs:  left edge at x=0, chip grows rightward  → x: [0 .. chipW]
    // Outputs: right edge at x=0, chip grows leftward  → x: [-chipW .. 0]
    QRectF chipRect = isOut ? QRectF(-chipW, -halfH, chipW, chipH)
                            : QRectF(0, -halfH, chipW, chipH);

    chipPath = QPainterPath();
    chipPath.addRoundedRect(chipRect, kRadius, kRadius);

    // Place text inside the chip with kPad from the inner edge
    textItem->setPos(isOut ? (-chipW + kPad) : kPad, -halfH - 1);

    prepareGeometryChange();
}

auto PortItem::boundingRect() const -> QRectF
{
    return chipPath.boundingRect();
}

// Connection anchor — the connector dot in scene coords.
// Both input and output anchors sit at local (0, 0).
auto PortItem::sceneCenter() const -> QPointF
{
    return mapToScene(0, 0);
}

void PortItem::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setRenderHint(QPainter::Antialiasing);

    const QRectF rect = chipPath.boundingRect();
    const QColor base = background();
    const bool selected = isSelected();
    const QColor border = selected
                              ? QApplication::palette().color(QPalette::Highlight)
                              : base.darker(150);

    // Gradient fill using Item helper
    painter->setBrush(gradientBrush(rect));
    painter->setPen(QPen(border, selected ? 2.0 : 1.0));
    painter->drawPath(chipPath);
}

auto PortItem::itemChange(GraphicsItemChange change, const QVariant &value)
    -> QVariant
{
    if (change == ItemScenePositionHasChanged) {
        Q_EMIT scenePositionChanged(itemData.id, sceneCenter());
    }

    return Item::itemChange(change, value);
}

void PortItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    setSelected(true);

    QMenu menu;
    const QAction *disconnectAct = menu.addAction(
        QStringLiteral("Disconnect all links"));

    if (menu.exec(event->screenPos()) == disconnectAct) {
        Q_EMIT this->disconnectRequested(itemData.id);
    }
}
