// canvas/PatchbayView.cpp
#include "PatchbayView.h"
#include "PatchbayScene.h"

#include <QApplication>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QWheelEvent>

PatchbayView::PatchbayView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    // QGraphicsScene is GUI-thread-bound. QQuickPaintedItem may otherwise paint
    // on the scene graph render thread, which causes cross-thread warnings
    // (QBasicTimer::stop) and instability when calling scene.render().
    setRenderTarget(QQuickPaintedItem::Image);

    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(QQuickItem::ItemAcceptsInputMethod);
    setFlag(QQuickItem::ItemIsFocusScope);
    setFocus(true);
    setFillColor(QApplication::palette().color(QPalette::Window));

    connect(&scene,
            &PatchbayScene::nodeActivated,
            this,
            &PatchbayView::nodeActivated);
    connect(&scene,
            &PatchbayScene::linkCreateRequested,
            this,
            &PatchbayView::linkCreateRequested);
    connect(&scene,
            &PatchbayScene::linkDestroyRequested,
            this,
            &PatchbayView::linkDestroyRequested);
    connect(&scene,
            &PatchbayScene::changed,
            this,
            [this](const QList<QRectF> &) { update(); });
}

void PatchbayView::setPatchbayModel(PatchbayModel *model)
{
    scene.setModel(model);
    update();
}

// ── QML property ───────────────────────────────────────────────────────────

void PatchbayView::setConnected(bool connected)
{
    if (isConnected == connected) {
        return;
    }
    isConnected = connected;
    Q_EMIT connectedChanged();
}

// ── Invokables ─────────────────────────────────────────────────────────────

void PatchbayView::resetLayout()
{
    scene.resetLayout();
    update();
}

void PatchbayView::resetView()
{
    offset = {0, 0};
    scale = 1.0;
    update();
}

// ── Paint ──────────────────────────────────────────────────────────────────

void PatchbayView::paint(QPainter *painter)
{
    // The visible portion of the scene in scene coordinates.
    QRectF source(-offset.x() / scale,
                  -offset.y() / scale,
                  width() / scale,
                  height() / scale);

    // Target is the full QQuickPaintedItem area.
    QRectF target(0, 0, width(), height());

    scene.render(painter, target, source);
}

// ── Geometry ───────────────────────────────────────────────────────────────

void PatchbayView::geometryChange(const QRectF &newGeometry,
                                  const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    scene.setSceneRect(0,
                       0,
                       qMax(newGeometry.width() / scale * 4, 4000.0),
                       qMax(newGeometry.height() / scale * 4, 4000.0));
    update();
}

// ── Coordinate helpers ─────────────────────────────────────────────────────

auto PatchbayView::_toScene(QPointF itemPos) const -> QPointF
{
    return (itemPos - offset) / scale;
}

// ── Mouse events ───────────────────────────────────────────────────────────

void PatchbayView::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) != 0) {
        // Zoom centred under the cursor
        QPointF cursor = event->position();
        qreal factor = event->angleDelta().y() > 0 ? 1.12 : (1.0 / 1.12);
        qreal newScale = qBound(kMinScale, scale * factor, kMaxScale);
        qreal ratio = newScale / scale;
        offset = cursor - ratio * (cursor - offset);
        scale = newScale;
        update();
        event->accept();
    } else {
        // Pan — one wheel step (120 units) = ~60px, natural scroll direction
        const QPointF delta = event->pixelDelta().isNull()
                                  ? event->angleDelta() / 2.0
                                  : event->pixelDelta();
        offset += delta;
        update();
        event->accept();
    }
}

void PatchbayView::mousePressEvent(QMouseEvent *event)
{
    // Ensure keyboard actions (eg. Delete) keep targeting this view/scene
    // after pointer interaction.
    forceActiveFocus(Qt::MouseFocusReason);

    if (event->button() == Qt::MiddleButton) {
        panning = true;
        panStart = event->position();
        offsetAtPanStart = offset;
        event->accept();
        return;
    }

    // Deliver to scene
    {
        QPointF scenePos = _toScene(event->position());
        QPoint screenPos = event->globalPosition().toPoint();
        QGraphicsSceneMouseEvent sceneMouseEvent(
            QEvent::GraphicsSceneMousePress);
        sceneMouseEvent.setScenePos(scenePos);
        sceneMouseEvent.setLastScenePos(scenePos);
        sceneMouseEvent.setScreenPos(screenPos);
        sceneMouseEvent.setLastScreenPos(screenPos);
        sceneMouseEvent.setButtonDownScenePos(event->button(), scenePos);
        sceneMouseEvent.setButtonDownScreenPos(event->button(), screenPos);
        sceneMouseEvent.setButton(event->button());
        sceneMouseEvent.setButtons(event->buttons());
        sceneMouseEvent.setModifiers(event->modifiers());
        QApplication::sendEvent(&scene, &sceneMouseEvent);
        lastScenePos = scenePos;
        lastScreenPos = screenPos;
        buttonDownScenePos = scenePos;
        buttonDownScreenPos = screenPos;
        update();
    }
    event->accept();
}

void PatchbayView::mouseMoveEvent(QMouseEvent *event)
{
    if (panning) {
        offset = offsetAtPanStart + (event->position() - panStart);
        update();
        event->accept();
        return;
    }
    {
        QPointF scenePos = _toScene(event->position());
        QPoint screenPos = event->globalPosition().toPoint();
        QGraphicsSceneMouseEvent sceneMouseEvent(QEvent::GraphicsSceneMouseMove);
        sceneMouseEvent.setScenePos(scenePos);
        sceneMouseEvent.setLastScenePos(lastScenePos);
        sceneMouseEvent.setScreenPos(screenPos);
        sceneMouseEvent.setLastScreenPos(lastScreenPos);
        sceneMouseEvent.setButtonDownScenePos(Qt::LeftButton,
                                              buttonDownScenePos);
        sceneMouseEvent.setButtonDownScreenPos(Qt::LeftButton,
                                               buttonDownScreenPos);
        sceneMouseEvent.setButton(event->button());
        sceneMouseEvent.setButtons(event->buttons());
        sceneMouseEvent.setModifiers(event->modifiers());
        QApplication::sendEvent(&scene, &sceneMouseEvent);
        lastScenePos = scenePos;
        lastScreenPos = screenPos;
        update();
    }
    event->accept();
}

void PatchbayView::mouseReleaseEvent(QMouseEvent *event)
{
    if (panning && event->button() == Qt::MiddleButton) {
        panning = false;
        event->accept();
        return;
    }
    {
        QPointF scenePos = _toScene(event->position());
        QPoint screenPos = event->globalPosition().toPoint();
        QGraphicsSceneMouseEvent sceneMouseEvent(
            QEvent::GraphicsSceneMouseRelease);
        sceneMouseEvent.setScenePos(scenePos);
        sceneMouseEvent.setLastScenePos(lastScenePos);
        sceneMouseEvent.setScreenPos(screenPos);
        sceneMouseEvent.setLastScreenPos(lastScreenPos);
        sceneMouseEvent.setButtonDownScenePos(event->button(),
                                              buttonDownScenePos);
        sceneMouseEvent.setButtonDownScreenPos(event->button(),
                                               buttonDownScreenPos);
        sceneMouseEvent.setButton(event->button());
        sceneMouseEvent.setButtons(event->buttons());
        sceneMouseEvent.setModifiers(event->modifiers());
        QApplication::sendEvent(&scene, &sceneMouseEvent);
        lastScenePos = scenePos;
        lastScreenPos = screenPos;

        if (event->button() == Qt::RightButton) {
            QGraphicsSceneContextMenuEvent sceneContextEvent(
                QEvent::GraphicsSceneContextMenu);
            sceneContextEvent.setScenePos(scenePos);
            sceneContextEvent.setScreenPos(screenPos);
            sceneContextEvent.setModifiers(event->modifiers());
            sceneContextEvent.setReason(QGraphicsSceneContextMenuEvent::Mouse);
            QApplication::sendEvent(&scene, &sceneContextEvent);
        }

        update();
    }
    event->accept();
}

void PatchbayView::keyPressEvent(QKeyEvent *event)
{
    QApplication::sendEvent(&scene, event);
    update();
}
