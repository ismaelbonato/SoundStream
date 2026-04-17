#pragma once
// canvas/Item.h
//
// Single-inheritance base for all canvas graphics items.
// Derives from QGraphicsObject, which fuses QObject + QGraphicsItem into one
// class — giving us QPropertyAnimation, QGraphicsEffect, and Qt's memory model
// with no diamond/virtual-inheritance trickery.
//
// Concrete subclasses (NodeItem, PortItem, LinkItem) inherit only this class.
// They use Qt's QObject signal/slot mechanism for event wiring.

#include <QColor>
#include <QGraphicsObject>
#include <QLinearGradient>
#include <QRectF>

class QPainter;

class Item : public QGraphicsObject
{
    // Q_OBJECT is intentionally omitted here — moc doesn't need to process this
    // base; the concrete subclasses don't use signals either.
    // If you ever need a signal on a subclass just add Q_OBJECT there.

public:
    explicit Item(QGraphicsItem *parent = nullptr);
    ~Item() override = default;

    // ── Colour accessors ──────────────────────────────────────────────────

    void setForeground(const QColor &color) { foregroundColor = color; }
    [[nodiscard]] auto foreground() const -> QColor { return foregroundColor; }

    void setBackground(const QColor &color) { backgroundColor = color; }
    [[nodiscard]] auto background() const -> QColor { return backgroundColor; }

    // ── Paint helpers ─────────────────────────────────────────────────────

    // Vertical linear gradient over `rect` using the item's background colour.
    // lighter top → slightly darker bottom, matching qpwgraph's visual style.
    [[nodiscard]] auto gradientBrush(const QRectF &rect) const
        -> QLinearGradient;

    [[nodiscard]] auto effectiveBackground() const -> QColor;

    // Returns a foreground colour that contrasts well with effectiveBackground().
    [[nodiscard]] auto effectiveForeground() const -> QColor;

    // Paints a rounded card body with this item's gradient background.
    void paintCard(QPainter *painter,
                   const QRectF &rect,
                   qreal radius,
                   const QColor &fillColor,
                   const QColor &borderColor,
                   qreal borderWidth) const;

    // ── QGraphicsItem interface (subclasses must implement) ───────────────

    [[nodiscard]] auto boundingRect() const -> QRectF override = 0;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override
        = 0;

protected:
    // Sensible defaults — subclasses call setForeground/setBackground in ctors.
    QColor foregroundColor{0xee, 0xee, 0xee}; // near-white text
    QColor backgroundColor{0x3a, 0x7e, 0xbf}; // neutral blue
};
