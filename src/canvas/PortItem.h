#pragma once
// canvas/PortItem.h
// A port chip: rounded rectangle with a text label, matching qpwgraph style.
// Inputs are blue, outputs are green.
//
// Inherits Item (QGraphicsObject-derived) — no multiple inheritance.
// Emits Qt signals for scene wiring.

#include "Item.h"
#include "core/GraphTypes.h"
#include <QPointer>
#include <QString>

class QGraphicsTextItem;
class NodeItem;

class PortItem : public Item
{
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 2 };

    explicit PortItem(const PortData &itemData, NodeItem *parentNode);

    auto type() const -> int override { return Type; }

    [[nodiscard]] auto portId() const -> quint32 { return itemData.id; }
    [[nodiscard]] auto nodeId() const -> quint32 { return itemData.nodeId; }
    [[nodiscard]] auto direction() const -> QString
    {
        return QString::fromStdString(itemData.direction);
    }
    [[nodiscard]] auto mediaType() const -> QString
    {
        return QString::fromStdString(itemData.mediaType);
    }
    [[nodiscard]] auto name() const -> QString
    {
        return QString::fromStdString(itemData.name);
    }
    [[nodiscard]] auto data() const -> const PortData & { return itemData; }

    // Connection anchor in scene coordinates.
    auto sceneCenter() const -> QPointF;

Q_SIGNALS:
    void scenePositionChanged(quint32 portId, QPointF scenePos);
    void disconnectRequested(quint32 portId);

public:
    // ── QGraphicsItem interface ───────────────────────────────────────────
    auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

protected:
    auto itemChange(GraphicsItemChange change, const QVariant &value)
        -> QVariant override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    void buildShape();

    PortData itemData;
    QPointer<QGraphicsTextItem> textItem;
    QPainterPath chipPath;

    static constexpr qreal kPad = 4.0;
    static constexpr qreal kRadius = 4.0;
};
