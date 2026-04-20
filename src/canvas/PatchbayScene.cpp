// canvas/PatchbayScene.cpp
#include "PatchbayScene.h"
#include "GraphLayouter.h"
#include "LinkInitializer.h"
#include "LinkItem.h"
#include "LinkPathUpdater.h"
#include "NodeItem.h"
#include "NodeRemovalHelper.h"
#include "PortItem.h"

#include <memory>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QSet>
#include <qcontainerfwd.h>

namespace {
auto portFromItem(QGraphicsItem *item) -> PortItem *
{
    for (QGraphicsItem *current = item; current != nullptr;
         current = current->parentItem()) {
        if (auto *port = qgraphicsitem_cast<PortItem *>(current)) {
            return port;
        }
    }
    return nullptr;
}
} // namespace

PatchbayScene::PatchbayScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-2000, -2000, 8000, 8000);
}

PatchbayScene::~PatchbayScene()
{
    // Drop signal connections before scene-owned items start disappearing.
    setModel(nullptr);
}

// ── Model wiring ───────────────────────────────────────────────────────────

void PatchbayScene::_connectModelSignals()
{
    connect(patchbayModel,
            &PatchbayModel::nodeAdded,
            this,
            &PatchbayScene::onNodeAdded);
    connect(patchbayModel,
            &PatchbayModel::nodeRemoved,
            this,
            &PatchbayScene::onNodeRemoved);
    connect(patchbayModel,
            &PatchbayModel::portAdded,
            this,
            &PatchbayScene::onPortAdded);
    connect(patchbayModel,
            &PatchbayModel::portRemoved,
            this,
            &PatchbayScene::onPortRemoved);
    connect(patchbayModel,
            &PatchbayModel::linkAdded,
            this,
            &PatchbayScene::onLinkAdded);
    connect(patchbayModel,
            &PatchbayModel::linkRemoved,
            this,
            &PatchbayScene::onLinkRemoved);
}

void PatchbayScene::_populateFromModelSnapshot()
{
    // Populate from the current model snapshot.
    for (const auto &[id, n] : patchbayModel->nodes)
        onNodeAdded(n);
    for (const auto &[id, p] : patchbayModel->ports)
        onPortAdded(p);
    for (const auto &[id, l] : patchbayModel->links)
        onLinkAdded(l);
}

auto PatchbayScene::_createAndWireNodeItem(const NodeData &node)
    -> std::unique_ptr<NodeItem>
{
    auto item = std::make_unique<NodeItem>(node);
    connect(item.get(),
            &NodeItem::nodeActivated,
            this,
            &PatchbayScene::nodeActivated);
    connect(item.get(), &NodeItem::nodeMovedTo, this, [](quint32, QPointF) {
        // Ports notify their own scene-position changes; links update there.
    });
    return item;
}

auto PatchbayScene::_attachAndWirePortItem(NodeItem &nodeItem,
                                           const PortData &port)
    -> std::optional<std::reference_wrapper<PortItem>>
{
    nodeItem.addPort(port);
    auto portItemOpt = nodeItem.portItem(port.id);
    if (!portItemOpt)
        return std::nullopt;

    PortItem &portRef = portItemOpt->get();

    connect(&portRef,
            &PortItem::scenePositionChanged,
            this,
            &PatchbayScene::onPortMoved);
    connect(&portRef,
            &PortItem::disconnectRequested,
            this,
            [this](quint32 portId) {
                for (const auto &linkObs : itemIndex.links()) {
                    const LinkItem *link = linkObs.data();
                    if (!link) {
                        continue;
                    }

                    if (link->outputPortId() == portId
                        || link->inputPortId() == portId) {
                        Q_EMIT this->linkDestroyRequested(link->linkId());
                    }
                }
            });
    return portItemOpt;
}

auto PatchbayScene::_createAndWireLinkItem(const LinkData &link)
    -> std::unique_ptr<LinkItem>
{
    auto item = std::make_unique<LinkItem>(link);
    connect(item.get(),
            &LinkItem::removeRequested,
            this,
            [this](quint32 linkId) { Q_EMIT linkDestroyRequested(linkId); });
    LinkInitializer::initializeEndpoints(*item, itemIndex);
    return item;
}

auto PatchbayScene::_nextRowForNodeColumn(const QString &mediaClass) const
    -> int
{
    const int col = GraphLayouter::layoutColumn(mediaClass);
    int row = 0;
    for (const auto &nObs : itemIndex.nodes()) {
        const NodeItem *nodeItem = nObs.data();
        if (nodeItem == nullptr) {
            continue;
        }
        if (GraphLayouter::layoutColumn(
                QString::fromStdString(nodeItem->data().mediaClass))
            == col) {
            ++row;
        }
    }
    return row;
}

void PatchbayScene::_selectPort(PortItem &port,
                                Qt::KeyboardModifiers modifiers)
{
    if ((modifiers & Qt::ControlModifier) != 0) {
        port.setSelected(!port.isSelected());
    } else {
        clearSelection();
        port.setSelected(true);
    }
}

void PatchbayScene::_removePortFromAllNodes(quint32 portId)
{
    // NodeItem::removePort handles item destruction.
    for (const auto &nObs : itemIndex.nodes()) {
        NodeItem *nodeItem = nObs.data();
        if (nodeItem != nullptr) {
            nodeItem->removePort(portId);
        }
    }
}

void PatchbayScene::setModel(PatchbayModel *model)
{
    if (patchbayModel == model) {
        return;
    }

    if (patchbayModel) {
        disconnect(patchbayModel, nullptr, this, nullptr);
    }

    patchbayModel = model;
    if (!patchbayModel) {
        return;
    }

    _connectModelSignals();
    _populateFromModelSnapshot();
}

// ── Model slot handlers ────────────────────────────────────────────────────

void PatchbayScene::onNodeAdded(NodeData node)
{
    if (itemIndex.hasNode(node.id)) {
        return;
    }

    auto item = _createAndWireNodeItem(node);
    NodeItem *itemRaw = item.get();

    // Initial position — resetLayout() may re-arrange later.
    const int col = GraphLayouter::layoutColumn(
        QString::fromStdString(node.mediaClass));
    const int row = _nextRowForNodeColumn(
        QString::fromStdString(node.mediaClass));
    itemRaw->setPos(GraphLayouter::gridPosition(col, row));

    addItem(itemRaw);
    itemIndex.insertNode(node.id, itemRaw);
    [[maybe_unused]] const NodeItem *releasedItem = item.release();
}

void PatchbayScene::onNodeRemoved(quint32 id)
{
    // Purge all PortItem pointers owned by this node before deleting it.
    // Without this, the port index may keep stale pointers and crash hit-testing.
    const NodeRemovalCleanup cleanup
        = NodeRemovalHelper::analyzePortCleanup(itemIndex,
                                                id,
                                                dragSourcePort.data());

    if (cleanup.clearsDragSource)
        dragSourcePort = nullptr;

    for (quint32 pid : cleanup.stalePortIds)
        itemIndex.removePort(pid);

    auto item = itemIndex.takeNode(id);
    if (item) {
        removeItem(item.data());
        delete item.data();
    }
}

void PatchbayScene::onPortAdded(PortData port)
{
    auto nodeItemOpt = itemIndex.node(port.nodeId);
    if (!nodeItemOpt)
        return;

    NodeItem &nodeRef = nodeItemOpt->get();

    auto pItemOpt = _attachAndWirePortItem(nodeRef, port);
    if (!pItemOpt)
        return;

    PortItem &portRef = pItemOpt->get();

    itemIndex.insertPort(port.id, &portRef);
}

void PatchbayScene::onPortRemoved(quint32 id)
{
    if (dragSourcePort && dragSourcePort->portId() == id)
        dragSourcePort = nullptr;

    itemIndex.removePort(id);
    _removePortFromAllNodes(id);
}

void PatchbayScene::onLinkAdded(LinkData link)
{
    if (itemIndex.hasLink(link.id))
        return;

    auto item = _createAndWireLinkItem(link);
    LinkItem *itemRaw = item.get();

    addItem(itemRaw);
    itemIndex.insertLink(link.id, itemRaw);
    [[maybe_unused]] const LinkItem *releasedItem = item.release();
}

void PatchbayScene::onLinkRemoved(quint32 id)
{
    auto item = itemIndex.takeLink(id);
    if (item) {
        removeItem(item.data());
        delete item.data();
    }
}

void PatchbayScene::onPortMoved(quint32 portId, QPointF /*scenePos*/)
{
    LinkPathUpdater::updateLinksForPort(portId, itemIndex);
}

// ── Layout ─────────────────────────────────────────────────────────────────

void PatchbayScene::resetLayout()
{
    QVector<QPair<quint32, QString>> layoutNodes;
    layoutNodes.reserve(itemIndex.nodes().size());
    for (const auto &itemObs : itemIndex.nodes()) {
        NodeItem *item = itemObs.data();
        if (!item)
            continue;
        layoutNodes.append(
            {item->nodeId(), QString::fromStdString(item->data().mediaClass)});
    }

    const QHash<quint32, QPointF> positions
        = GraphLayouter::computeGridPositions(layoutNodes);
    for (const auto &itemObs : itemIndex.nodes()) {
        NodeItem *item = itemObs.data();
        if (!item)
            continue;
        item->setPos(positions.value(item->nodeId(), item->pos()));
    }
}

// ── Background dot-grid ────────────────────────────────────────────────────

void PatchbayScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(128, 128, 128, 45));

    // Only draw dots inside the visible rect — scene handles clipping.
    qreal left = qFloor(rect.left() / kGridStep) * kGridStep;
    qreal top = qFloor(rect.top() / kGridStep) * kGridStep;
    qreal right = qCeil(rect.right() / kGridStep) * kGridStep;
    qreal bottom = qCeil(rect.bottom() / kGridStep) * kGridStep;

    for (qreal x = left; x <= right; x += kGridStep)
        for (qreal y = top; y <= bottom; y += kGridStep)
            painter->drawEllipse(QPointF(x, y), 1.5, 1.5);
}

void PatchbayScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    if (!draggingLink || !dragSourcePort)
        return;

    const QPointF src = dragSourcePort->sceneCenter();
    const QPointF dst = dragCurrentPos;
    const qreal off = qMax(60.0, qAbs(dst.x() - src.x()) * 0.45);

    QPainterPath previewPath;
    previewPath.moveTo(src);
    previewPath.cubicTo(src + QPointF(off, 0), dst - QPointF(off, 0), dst);

    painter->setRenderHint(QPainter::Antialiasing);

    const QColor accent = dragSourcePort->background();
    QColor glow = accent;
    glow.setAlpha(90);

    // Soft outer stroke for visibility on all backgrounds.
    painter->setPen(QPen(glow, 5.0, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(previewPath);

    // Crisp main stroke matching the source port color.
    painter->setPen(QPen(accent.lighter(120), 2.5, Qt::SolidLine, Qt::RoundCap));
    painter->drawPath(previewPath);
}

// ── Drag-to-link ───────────────────────────────────────────────────────────

void PatchbayScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        auto *item = itemAt(event->scenePos(), QTransform());
        if (auto *port = portFromItem(item)) {
            _selectPort(*port, event->modifiers());

            // Only output ports start a drag
            if (port->direction() == QLatin1String("out")
                && (event->modifiers() & Qt::ControlModifier) == 0) {
                dragSourcePort = port;
                dragCurrentPos = event->scenePos();
                draggingLink = true;
            }

            update();
            event->accept();
            return;
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void PatchbayScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (draggingLink) {
        dragCurrentPos = event->scenePos();
        update(); // triggers drawForeground → we draw rubber band via invalidate
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void PatchbayScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (draggingLink && event->button() == Qt::LeftButton) {
        draggingLink = false;
        auto *item = itemAt(event->scenePos(), QTransform());
        if (const auto *port = portFromItem(item)) {
            if (port->direction() == QLatin1String("in") && dragSourcePort) {
                Q_EMIT this->linkCreateRequested(dragSourcePort->portId(),
                                                 port->portId());
            }
        }
        dragSourcePort = nullptr;
        update();
        event->accept();
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void PatchbayScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        QSet<quint32> linkIdsToDestroy;

        for (auto *item : selectedItems()) {
            if (auto *link = qgraphicsitem_cast<LinkItem *>(item)) {
                linkIdsToDestroy.insert(link->linkId());
                continue;
            }

            if (auto *port = qgraphicsitem_cast<PortItem *>(item)) {
                const quint32 portId = port->portId();
                for (const auto &linkObs : itemIndex.links()) {
                    LinkItem *link = linkObs.data();
                    if (!link)
                        continue;

                    if (link->outputPortId() == portId
                        || link->inputPortId() == portId) {
                        linkIdsToDestroy.insert(link->linkId());
                    }
                }
            }
        }

        for (quint32 linkId : linkIdsToDestroy) {
            Q_EMIT this->linkDestroyRequested(linkId);
        }

        event->accept();
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}
