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

void PatchbayView::setZoom(qreal zoom)
{
    _setZoomAround(zoom, QPointF(width() / 2.0, height() / 2.0));
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
    const bool zoomWasChanged = !qFuzzyCompare(scale, 1.0);
    scale = 1.0;
    _updateSceneRect();
    update();
    if (zoomWasChanged) {
        Q_EMIT zoomChanged();
    }
}

void PatchbayView::zoomIn()
{
    setZoom(scale * 1.12);
}

void PatchbayView::zoomOut()
{
    setZoom(scale / 1.12);
}

void PatchbayView::centerView()
{
    const QRectF itemBounds = scene.itemsBoundingRect();
    const QPointF sceneCenter = itemBounds.isValid() && !itemBounds.isEmpty()
                                    ? itemBounds.center()
                                    : scene.sceneRect().center();
    offset = QPointF(width() / 2.0, height() / 2.0) - sceneCenter * scale;
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
    _updateSceneRect();
    update();
}

// ── Coordinate helpers ─────────────────────────────────────────────────────

auto PatchbayView::_toScene(QPointF itemPos) const -> QPointF
{
    return (itemPos - offset) / scale;
}

void PatchbayView::_setZoomAround(qreal zoom, QPointF anchor)
{
    const qreal newScale = qBound(kMinScale, zoom, kMaxScale);
    if (qFuzzyCompare(scale, newScale)) {
        return;
    }

    const qreal ratio = newScale / scale;
    offset = anchor - ratio * (anchor - offset);
    scale = newScale;
    _updateSceneRect();
    update();
    Q_EMIT zoomChanged();
}

void PatchbayView::_updateSceneRect()
{
    scene.setSceneRect(0,
                       0,
                       qMax(width() / scale * 4, 4000.0),
                       qMax(height() / scale * 4, 4000.0));
}

// ── Mouse events ───────────────────────────────────────────────────────────

void PatchbayView::wheelEvent(QWheelEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) != 0) {
        // Zoom centred under the cursor
        const qreal factor = event->angleDelta().y() > 0 ? 1.12 : (1.0 / 1.12);
        _setZoomAround(scale * factor, event->position());
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
