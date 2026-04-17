#pragma once
// canvas/LinkItem.h
// Renders one Bézier connection between two PortItems.
//
// Inherits Item (QGraphicsObject-derived) — no multiple inheritance.
// Emits Qt signals for scene wiring.

#include "Item.h"
#include "core/GraphTypes.h"

#include <QString>

class LinkItem : public Item
{
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 3 };

    explicit LinkItem(const LinkData &itemData, QGraphicsItem *parent = nullptr);

    [[nodiscard]] auto type() const -> int override { return Type; }

    [[nodiscard]] auto linkId() const -> quint32 { return itemData.id; }
    [[nodiscard]] auto outputPortId() const -> quint32
    {
        return itemData.outputPortId;
    }
    [[nodiscard]] auto inputPortId() const -> quint32
    {
        return itemData.inputPortId;
    }
    [[nodiscard]] auto data() const -> LinkData { return itemData; }

    void setMediaType(QString mediaType);

    // Update endpoints — called whenever a connected PortItem moves.
    void setEndpoints(QPointF src, QPointF dst);

    // QGraphicsItem::shape() override: returns a thick stroke path so that
    // click hit-testing follows the curve rather than its bounding box.
    [[nodiscard]] auto shape() const -> QPainterPath override;

Q_SIGNALS:
    void removeRequested(quint32 linkId);

public:
    // ── QGraphicsItem interface ───────────────────────────────────────────
    [[nodiscard]] auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    auto itemChange(GraphicsItemChange change, const QVariant &value)
        -> QVariant override;

private:
    void _rebuildPath(QPointF src, QPointF dst);
    void _refreshColors();

    LinkData itemData;
    QString mediaType;
    QPointF srcPt;
    QPointF dstPt;
    QPainterPath linkPath;

    static constexpr qreal kHitWidth = 12.0; // click tolerance in px
    static constexpr qreal kLineWidth = 2.0;
};
