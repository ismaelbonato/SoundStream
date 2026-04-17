#pragma once
// canvas/NodeItem.h
// Draggable card representing one node in the graph.
// Owns its PortItems and lays them out automatically.
//
// Inherits Item (QGraphicsObject-derived) — no multiple inheritance.
// Emits Qt signals for scene wiring.

#include "Item.h"
#include "core/GraphTypes.h"

#include <QHash>
#include <QPointer>

#include <functional>
#include <optional>

class PortItem;

class NodeItem : public Item
{
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 1 };

    explicit NodeItem(const NodeData &itemData, QGraphicsItem *parent = nullptr);

    [[nodiscard]] auto type() const -> int override { return Type; }

    [[nodiscard]] auto nodeId() const -> quint32 { return itemData.id; }
    [[nodiscard]] auto data() const -> const NodeData & { return itemData; }

    void addPort(const PortData &port);
    void removePort(quint32 portId);
    [[nodiscard]] auto portItem(quint32 portId) const
        -> std::optional<std::reference_wrapper<PortItem>>;
    void layoutPorts();

Q_SIGNALS:
    void nodeMovedTo(quint32 nodeId, QPointF scenePos);
    void nodeActivated(quint32 nodeId);

public:
    // ── QGraphicsItem interface ───────────────────────────────────────────
    [[nodiscard]] auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

protected:
    auto itemChange(GraphicsItemChange change, const QVariant &value)
        -> QVariant override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    static auto _classColor(const QString &mediaClass) -> QColor;

    NodeData itemData;
    QHash<quint32, QPointer<PortItem>> ports;
    QPointF dragOffset;
    QRectF rect;

    static constexpr qreal kMinWidth = 200.0;
    static constexpr qreal kMinHeight = 52.0;
    static constexpr qreal kPortSpacing = 18.0;
    static constexpr qreal kHeaderH = 20.0;
    static constexpr qreal kRadius = 6.0;
};
