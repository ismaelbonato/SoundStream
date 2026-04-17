// canvas/Item.cpp
#include "Item.h"

#include <QPainter>
#include <QPen>

Item::Item(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
}

QLinearGradient Item::gradientBrush(const QRectF &rect) const
{
    const QColor base = backgroundColor;
    QLinearGradient grad(0, rect.top(), 0, rect.bottom());
    grad.setColorAt(0.0, base.lighter(120));
    grad.setColorAt(1.0, base.darker(130));
    return grad;
}

QColor Item::effectiveBackground() const
{
    return backgroundColor;
}

QColor Item::effectiveForeground() const
{
    const bool isDark = (backgroundColor.value() < 128);
    return isDark ? foregroundColor.lighter(130) : foregroundColor.darker(130);
}

void Item::paintCard(QPainter *painter,
                     const QRectF &rect,
                     qreal radius,
                     const QColor &fillColor,
                     const QColor &borderColor,
                     qreal borderWidth) const
{
    QLinearGradient gradient(0, rect.top(), 0, rect.bottom());
    gradient.setColorAt(0.0, fillColor.lighter(120));
    gradient.setColorAt(1.0, fillColor.darker(130));

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(gradient);
    painter->setPen(QPen(borderColor, borderWidth));
    painter->drawRoundedRect(rect, radius, radius);
}
