// canvas/NodeItem.cpp
#include "NodeItem.h"
#include "PortItem.h"

#include <QApplication>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>
#include <qcontainerfwd.h>

NodeItem::NodeItem(const NodeData &itemData, QGraphicsItem *parent)
    : Item(parent)
    , itemData(itemData)
    , rect(0, 0, kMinWidth, kMinHeight)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setZValue(1);
    setToolTip(QString::fromStdString(itemData.name));

    // Initialise Item colours from the system palette
    const QPalette &pal = QApplication::palette();
    setForeground(pal.color(QPalette::Text));
    setBackground(pal.color(QPalette::Base));

    // Apply initial auto-sizing even when there are no ports yet
    // (eg. long node titles should expand width on creation).
    layoutPorts();
}

// ── Port management ────────────────────────────────────────────────────────

void NodeItem::addPort(const PortData &port)
{
    if (ports.contains(port.id)) {
        return;
    }
    auto *item = new PortItem(port, this); // scene takes ownership via parent
    ports.insert(port.id, item);
    layoutPorts();
}

void NodeItem::removePort(quint32 portId)
{
    auto item = ports.take(portId);
    // QGraphicsItem destructor automatically removes the item from the
    // scene and from this parent item's children — no manual removeItem needed.
    delete item;
    layoutPorts();
}

auto NodeItem::portItem(quint32 portId) const
    -> std::optional<std::reference_wrapper<PortItem>>
{
    if (auto *item = ports.value(portId, nullptr).data()) {
        return std::ref(*item);
    }

    return std::nullopt;
}

// ── QGraphicsItem interface ─────────────────────────────────────────────────

auto NodeItem::boundingRect() const -> QRectF
{
    return rect;
}

void NodeItem::layoutPorts()
{
    static constexpr qreal kBottomPad = 6.0;
    static constexpr qreal kLaneGap = 20.0;
    static constexpr qreal kHeaderLeftPad = 28.0;
    static constexpr qreal kHeaderRightPad = 8.0;

    QList<PortItem *> outs;
    QList<PortItem *> ins;

    for (const auto &port : std::as_const(ports)) {
        if (port.isNull()) {
            continue;
        }
        if (port->direction() == QLatin1String("out")) {
            outs.append(port.data());
        } else {
            ins.append(port.data());
        }
    }

    const qsizetype inputRows = std::max<qsizetype>(ins.size(), 1);
    const qsizetype outputRows = std::max<qsizetype>(outs.size(), 1);
    const qsizetype laneRows = std::max<qsizetype>(inputRows, outputRows);

    QFont titleFont = QApplication::font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(titleFont.pointSizeF() * 0.75);
    const QFontMetricsF titleFm(titleFont);

    const qreal nameWidth = titleFm.horizontalAdvance(
        QString::fromStdString(itemData.name));
    const qreal requiredWidthFromName = kHeaderLeftPad + nameWidth
                                        + kHeaderRightPad;

    qreal maxInputInward = 0.0;
    for (const PortItem *port : ins) {
        maxInputInward = qMax(maxInputInward, port->boundingRect().right());
    }

    qreal maxOutputInward = 0.0;
    for (const PortItem *port : outs) {
        maxOutputInward = qMax(maxOutputInward, -port->boundingRect().left());
    }

    const qreal requiredWidthFromPorts = maxInputInward + maxOutputInward
                                         + kLaneGap;
    const qreal nodeWidth
        = qMax(kMinWidth, qMax(requiredWidthFromName, requiredWidthFromPorts));

    qreal nodeHeight = kHeaderH + (static_cast<qreal>(laneRows) * kPortSpacing)
                       + kBottomPad;
    nodeHeight = qMax(nodeHeight, kMinHeight);

    prepareGeometryChange();
    rect = QRectF(0, 0, nodeWidth, nodeHeight);

    auto layoutSide = [](const QList<PortItem *> &sidePorts, qreal xAnchor) {
        for (int i = 0; i < sidePorts.size(); ++i) {
            sidePorts[i]->setPos(xAnchor, kHeaderH + ((i + 0.5) * kPortSpacing));
        }
    };

    // Inputs on left lane, outputs on right lane.
    layoutSide(ins, 0);
    layoutSide(outs, nodeWidth);
}

// ── Rendering ──────────────────────────────────────────────────────────────

void NodeItem::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    const QRectF localRect = rect;
    const bool selected = isSelected();

    // Card body — shared base drawing from Item.
    QColor accent = _classColor(QString::fromStdString(itemData.mediaClass));
    accent.setAlpha(220);

    QColor base = effectiveBackground();
    base.setAlpha(220);

    QColor border = selected
                        ? QApplication::palette().color(QPalette::Highlight)
                        : QApplication::palette().color(QPalette::Mid);

    paintCard(painter, localRect, kRadius, base, border, selected ? 2.0 : 1.0);

    // Header stripe — clipped to card's top corners
    QRectF header(localRect.x(), localRect.y(), localRect.width(), kHeaderH);
    QPainterPath clip;
    clip.addRoundedRect(localRect, kRadius, kRadius);
    painter->setClipPath(clip);
    painter->setBrush(accent);
    painter->setPen(Qt::NoPen);
    painter->drawRect(header);
    painter->setClipping(false);

    // Node name
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSizeF(font.pointSizeF() * 0.75);
    painter->setFont(font);
    QRectF textRect(localRect.x() + 28,
                    localRect.y(),
                    localRect.width() - 32,
                    kHeaderH);
    painter->drawText(textRect,
                      Qt::AlignVCenter | Qt::AlignLeft,
                      painter->fontMetrics().elidedText(QString::fromStdString(
                                                            itemData.name),
                                                        Qt::ElideRight,
                                                        int(textRect.width())));

    // Icon placeholder — small white dot on header left
    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(localRect.x() + 14,
                                 localRect.y() + (kHeaderH / 2)),
                         6,
                         6);
}

auto NodeItem::_classColor(const QString &mediaClass) -> QColor
{
    if (mediaClass.contains(QLatin1String("Sink"), Qt::CaseInsensitive)) {
        return {0x3a, 0x7e, 0xbf};
    }
    if (mediaClass.contains(QLatin1String("Source"), Qt::CaseInsensitive)) {
        return {0x3a, 0xbf, 0x6e};
    }
    if (mediaClass.contains(QLatin1String("Duplex"), Qt::CaseInsensitive)) {
        return {0x7e, 0x3a, 0xbf};
    }
    return QColor(0x5a, 0x5a, 0x7a);
}

// ── Events ─────────────────────────────────────────────────────────────────

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragOffset = event->scenePos() - pos();
        setSelected(true);
        setZValue(2);
        event->accept();
        return;
    }
    Item::mousePressEvent(event);
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) != 0) {
        setPos(event->scenePos() - dragOffset);
        event->accept();
        return;
    }
    Item::mouseMoveEvent(event);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setZValue(1);
    }

    Item::mouseReleaseEvent(event);
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        Q_EMIT nodeMovedTo(itemData.id, value.toPointF());
    }

    return Item::itemChange(change, value);
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    Q_EMIT nodeActivated(itemData.id);
}
