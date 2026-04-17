#pragma once
// canvas/PatchbayView.h
// QQuickPaintedItem that hosts a QGraphicsScene directly in the Qt Quick
// scene graph.  No native window embedding — paints via QPainter into a
// texture that Qt Quick composites with the rest of the QML scene.
//
// Zoom: Ctrl+scroll  Pan: middle-mouse or left-drag on background
// Registered as QML_ELEMENT so Main.qml can use "PatchbayView { }".

#include "PatchbayModel.h"
#include "PatchbayScene.h"

#include <QGraphicsScene>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

class PatchbayView : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY
                   connectedChanged)

public:
    explicit PatchbayView(QQuickItem *parent = nullptr);

    void setPatchbayModel(PatchbayModel *model);

    [[nodiscard]] auto connected() const -> bool { return isConnected; }
    void setConnected(bool connected);

    Q_INVOKABLE void resetLayout();
    Q_INVOKABLE void resetView();

    // QQuickPaintedItem
    void paint(QPainter *painter) override;

Q_SIGNALS:
    void nodeActivated(quint32 nodeId);
    void connectedChanged();
    void linkCreateRequested(quint32 outputPortId, quint32 inputPortId);
    void linkDestroyRequested(quint32 linkId);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void geometryChange(const QRectF &newGeometry,
                        const QRectF &oldGeometry) override;

private:
    // Convert a QQuickItem mouse-event position to scene coordinates.
    auto _toScene(QPointF itemPos) const -> QPointF;

    PatchbayScene scene;

    // View transform: pan offset + zoom scale (applied in paint()).
    QPointF offset = {0, 0};
    qreal scale = 1.0;

    // Pan state
    bool panning = false;
    QPointF panStart;
    QPointF offsetAtPanStart;

    // Last delivered positions — used to fill last* fields in move/release events.
    QPointF lastScenePos;
    QPoint lastScreenPos;

    // Button-down scene position — must be forwarded on every move event.
    QPointF buttonDownScenePos;
    QPoint buttonDownScreenPos;

    bool isConnected = false;

    static constexpr qreal kMinScale = 0.2;
    static constexpr qreal kMaxScale = 4.0;
};
