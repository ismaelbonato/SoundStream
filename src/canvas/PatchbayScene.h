#pragma once
// canvas/PatchbayScene.h
// QGraphicsScene subclass that owns NodeItems, PortItems and LinkItems.
// Driven entirely by PatchbayModel callbacks — knows nothing about PipeWire.

#include "PatchbayModel.h"
#include "SceneItemIndex.h"

#include <QGraphicsScene>
#include <QPointer>
#include <functional>
#include <memory>
#include <optional>

class NodeItem;
class PortItem;
class LinkItem;

class PatchbayScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit PatchbayScene(QObject *parent = nullptr);
    ~PatchbayScene() override;

    void setModel(PatchbayModel *model);

    // Arrange nodes in a grid — callable from QML via PatchbayView.
    Q_INVOKABLE void resetLayout();

Q_SIGNALS:
    // Forwarded from NodeItem double-click.
    void nodeActivated(quint32 nodeId);

    // Emitted when the user completes a drag-to-link gesture.
    void linkCreateRequested(quint32 outputPortId, quint32 inputPortId);

    // Emitted when the user deletes a link (right-click remove or Delete key).
    void linkDestroyRequested(quint32 linkId);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)  override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void _connectModelSignals();
    void _populateFromModelSnapshot();
    auto _createAndWireNodeItem(const NodeData &node)
        -> std::unique_ptr<NodeItem>;
    auto _attachAndWirePortItem(NodeItem &nodeItem, const PortData &port)
        -> std::optional<std::reference_wrapper<PortItem>>;
    auto _createAndWireLinkItem(const LinkData &link)
        -> std::unique_ptr<LinkItem>;
    auto _nextRowForNodeColumn(const QString &mediaClass) const -> int;
    void _selectPort(PortItem &port, Qt::KeyboardModifiers modifiers);
    void _removePortFromAllNodes(quint32 portId);

    // Slots/handlers for PatchbayModel signals.
    void onNodeAdded(NodeData node);
    void onNodeRemoved(quint32 id);
    void onPortAdded(PortData port);
    void onPortRemoved(quint32 id);
    void onLinkAdded(LinkData link);
    void onLinkRemoved(quint32 id);

    // PortItem position changed → update connected links
    void onPortMoved(quint32 portId, QPointF scenePos);

    QPointer<PatchbayModel> patchbayModel;  // non-owning
    SceneItemIndex itemIndex;

    // Drag-to-link rubber band state.
    QPointer<PortItem>  dragSourcePort;
    QPointF             dragCurrentPos;
    bool                draggingLink    = false;

    static constexpr qreal kGridStep = 24.0;
};
