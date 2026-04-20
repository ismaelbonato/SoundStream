#include "canvas/NodeItem.h"
#include "canvas/PatchbayModel.h"
#include "canvas/PatchbayScene.h"
#include "canvas/PortItem.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QTest>

#include <functional>
#include <optional>

namespace {
void sendMouseEvent(PatchbayScene &scene,
                    QEvent::Type type,
                    const QPointF &scenePos,
                    Qt::MouseButton button,
                    Qt::MouseButtons buttons)
{
    QGraphicsSceneMouseEvent ev(type);
    ev.setScenePos(scenePos);
    ev.setLastScenePos(scenePos);
    ev.setButtonDownScenePos(button, scenePos);
    ev.setButton(button);
    ev.setButtons(buttons);
    ev.setModifiers(Qt::NoModifier);
    QApplication::sendEvent(&scene, &ev);
}

auto findPort(PatchbayScene &scene, quint32 portId)
    -> std::optional<std::reference_wrapper<PortItem>>
{
    for (QGraphicsItem *item : scene.items()) {
        if (auto *port = qgraphicsitem_cast<PortItem *>(item);
            port && port->portId() == portId) {
            return std::ref(*port);
        }
    }
    return std::nullopt;
}

auto findNode(PatchbayScene &scene, quint32 nodeId)
    -> std::optional<std::reference_wrapper<NodeItem>>
{
    for (QGraphicsItem *item : scene.items()) {
        if (auto *node = qgraphicsitem_cast<NodeItem *>(item);
            node && node->nodeId() == nodeId) {
            return std::ref(*node);
        }
    }
    return std::nullopt;
}

template<typename T>
auto ref(std::optional<std::reference_wrapper<T>> &valueOpt) -> T &
{
    return valueOpt->get();
}
} // namespace

class PatchbaySceneInteractionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void dragOutputToInputEmitsLinkCreateRequested();
    void dragNodeChangesPosition();
    void draggedNodeStaysAboveOverlappingNodes();
    void clickingInputSelectsPort();
    void clickingOutputSelectsPortAndKeepsDragToLinkAvailable();
    void deleteKeyOnSelectedPortEmitsLinkDestroyRequested();
    void nodeResizesWhenNameIsLong();
    void nodeResizesForWidePortItems();
};

void PatchbaySceneInteractionTest::dragOutputToInputEmitsLinkCreateRequested()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 1,
                 .name = std::string("Source"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});
    model.notifyNodeAdded(NodeData{.id = 2,
                                   .name = std::string("Sink"),
                                   .mediaClass = std::string("Audio/Sink"),
                                   .iconName = std::string("audio-card")});
    model.notifyPortAdded(PortData{.id = 100,
                                   .nodeId = 1,
                                   .name = std::string("out"),
                                   .direction = std::string("out"),
                                   .mediaType = std::string("audio")});
    model.notifyPortAdded(PortData{.id = 200,
                                   .nodeId = 2,
                                   .name = std::string("in"),
                                   .direction = std::string("in"),
                                   .mediaType = std::string("audio")});

    auto outPortOpt = findPort(scene, 100);
    auto inPortOpt = findPort(scene, 200);
    QVERIFY(outPortOpt.has_value());
    QVERIFY(inPortOpt.has_value());

    PortItem &outRef = ref(outPortOpt);
    PortItem &inRef = ref(inPortOpt);

    QSignalSpy spy(&scene, &PatchbayScene::linkCreateRequested);

    const QPointF from = outRef.mapToScene(outRef.boundingRect().center());
    const QPointF to = inRef.mapToScene(inRef.boundingRect().center());

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMousePress,
                   from,
                   Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseMove,
                   to,
                   Qt::NoButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseRelease,
                   to,
                   Qt::LeftButton,
                   Qt::NoButton);

    QCOMPARE(spy.count(), 1);

    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toUInt(), 100u);
    QCOMPARE(args.at(1).toUInt(), 200u);
}

void PatchbaySceneInteractionTest::dragNodeChangesPosition()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 10,
                 .name = std::string("Movable"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});

    auto nodeOpt = findNode(scene, 10);
    QVERIFY(nodeOpt.has_value());

    NodeItem &nodeRef = ref(nodeOpt);

    QSignalSpy movedSpy(&nodeRef, &NodeItem::nodeMovedTo);

    const QPointF startPos = nodeRef.scenePos() + QPointF(20.0, 20.0);
    const QPointF endPos = startPos + QPointF(60.0, 40.0);

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMousePress,
                   startPos,
                   Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseMove,
                   endPos,
                   Qt::NoButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseRelease,
                   endPos,
                   Qt::LeftButton,
                   Qt::NoButton);

    QVERIFY(movedSpy.count() > 0);
    QVERIFY(nodeRef.scenePos() != QPointF(0.0, 0.0));
}

void PatchbaySceneInteractionTest::draggedNodeStaysAboveOverlappingNodes()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 100,
                 .name = std::string("Bottom"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});
    model.notifyNodeAdded(NodeData{.id = 200,
                                   .name = std::string("TopWhenDragging"),
                                   .mediaClass = std::string("Audio/Sink"),
                                   .iconName = std::string("audio-card")});

    auto firstNodeOpt = findNode(scene, 100);
    auto draggedNodeOpt = findNode(scene, 200);
    QVERIFY(firstNodeOpt.has_value());
    QVERIFY(draggedNodeOpt.has_value());

    NodeItem &firstNodeRef = ref(firstNodeOpt);
    NodeItem &draggedNodeRef = ref(draggedNodeOpt);

    firstNodeRef.setPos(QPointF(150.0, 150.0));
    draggedNodeRef.setPos(QPointF(180.0, 170.0));

    const QPointF dragStart = draggedNodeRef.scenePos() + QPointF(20.0, 20.0);
    const QPointF dragEnd = firstNodeRef.scenePos() + QPointF(30.0, 30.0);

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMousePress,
                   dragStart,
                   Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseMove,
                   dragEnd,
                   Qt::NoButton,
                   Qt::LeftButton);

    QVERIFY(draggedNodeRef.zValue() > firstNodeRef.zValue());

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseRelease,
                   dragEnd,
                   Qt::LeftButton,
                   Qt::NoButton);
}

void PatchbaySceneInteractionTest::clickingInputSelectsPort()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(NodeData{.id = 1,
                                   .name = std::string("Sink"),
                                   .mediaClass = std::string("Audio/Sink"),
                                   .iconName = std::string("audio-card")});
    model.notifyPortAdded(PortData{.id = 59,
                                   .nodeId = 1,
                                   .name = std::string("in"),
                                   .direction = std::string("in"),
                                   .mediaType = std::string("audio")});

    auto portOpt = findPort(scene, 59);
    QVERIFY(portOpt.has_value());

    PortItem &portRef = ref(portOpt);
    const QPointF clickPos = portRef.mapToScene(portRef.boundingRect().center());

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMousePress,
                   clickPos,
                   Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseRelease,
                   clickPos,
                   Qt::LeftButton,
                   Qt::NoButton);

    QVERIFY(portRef.isSelected());
}

void PatchbaySceneInteractionTest::
    clickingOutputSelectsPortAndKeepsDragToLinkAvailable()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 1,
                 .name = std::string("Source"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});
    model.notifyNodeAdded(NodeData{.id = 2,
                                   .name = std::string("Sink"),
                                   .mediaClass = std::string("Audio/Sink"),
                                   .iconName = std::string("audio-card")});
    model.notifyPortAdded(PortData{.id = 132,
                                   .nodeId = 1,
                                   .name = std::string("out"),
                                   .direction = std::string("out"),
                                   .mediaType = std::string("audio")});
    model.notifyPortAdded(PortData{.id = 59,
                                   .nodeId = 2,
                                   .name = std::string("in"),
                                   .direction = std::string("in"),
                                   .mediaType = std::string("audio")});

    auto outPortOpt = findPort(scene, 132);
    auto inPortOpt = findPort(scene, 59);
    QVERIFY(outPortOpt.has_value());
    QVERIFY(inPortOpt.has_value());

    PortItem &outRef = ref(outPortOpt);
    PortItem &inRef = ref(inPortOpt);

    QSignalSpy spy(&scene, &PatchbayScene::linkCreateRequested);

    const QPointF from = outRef.mapToScene(outRef.boundingRect().center());
    const QPointF to = inRef.mapToScene(inRef.boundingRect().center());

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMousePress,
                   from,
                   Qt::LeftButton,
                   Qt::LeftButton);
    QVERIFY(outRef.isSelected());

    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseMove,
                   to,
                   Qt::NoButton,
                   Qt::LeftButton);
    sendMouseEvent(scene,
                   QEvent::GraphicsSceneMouseRelease,
                   to,
                   Qt::LeftButton,
                   Qt::NoButton);

    QCOMPARE(spy.count(), 1);
}

void PatchbaySceneInteractionTest::
    deleteKeyOnSelectedPortEmitsLinkDestroyRequested()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 1,
                 .name = std::string("Source"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});
    model.notifyNodeAdded(NodeData{.id = 2,
                                   .name = std::string("Sink"),
                                   .mediaClass = std::string("Audio/Sink"),
                                   .iconName = std::string("audio-card")});
    model.notifyPortAdded(PortData{.id = 132,
                                   .nodeId = 1,
                                   .name = std::string("out"),
                                   .direction = std::string("out"),
                                   .mediaType = std::string("audio")});
    model.notifyPortAdded(PortData{.id = 59,
                                   .nodeId = 2,
                                   .name = std::string("in"),
                                   .direction = std::string("in"),
                                   .mediaType = std::string("audio")});
    model.notifyLinkAdded(LinkData{.id = 501,
                                   .outputPortId = 132,
                                   .inputPortId = 59,
                                   .active = true});

    auto portOpt = findPort(scene, 132);
    QVERIFY(portOpt.has_value());

    PortItem &portRef = ref(portOpt);
    portRef.setSelected(true);

    QSignalSpy spy(&scene, &PatchbayScene::linkDestroyRequested);

    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QApplication::sendEvent(&scene, &keyEvent);

    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toUInt(), 501u);
}

void PatchbaySceneInteractionTest::nodeResizesWhenNameIsLong()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 300,
                 .name = std::string("Short"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});
    model.notifyNodeAdded(
        NodeData{.id = 301,
                 .name = std::string("This is a very very long node name "
                                     "that should force width growth"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});

    auto shortNodeOpt = findNode(scene, 300);
    auto longNodeOpt = findNode(scene, 301);
    QVERIFY(shortNodeOpt.has_value());
    QVERIFY(longNodeOpt.has_value());

    NodeItem &shortNodeRef = ref(shortNodeOpt);
    NodeItem &longNodeRef = ref(longNodeOpt);

    QVERIFY(longNodeRef.boundingRect().width()
            > shortNodeRef.boundingRect().width());
}

void PatchbaySceneInteractionTest::nodeResizesForWidePortItems()
{
    PatchbayModel model;
    PatchbayScene scene;
    scene.setModel(&model);

    model.notifyNodeAdded(
        NodeData{.id = 400,
                 .name = std::string("Baseline"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});
    model.notifyNodeAdded(
        NodeData{.id = 401,
                 .name = std::string("Ports"),
                 .mediaClass = std::string("Audio/Source"),
                 .iconName = std::string("audio-input-microphone")});

    model.notifyPortAdded(
        PortData{.id = 410,
                 .nodeId = 401,
                 .name = std::string(
                     "capture: this is a very very wide input port label"),
                 .direction = std::string("in"),
                 .mediaType = std::string("audio")});
    model.notifyPortAdded(
        PortData{.id = 411,
                 .nodeId = 401,
                 .name = std::string(
                     "playback: this is a very very wide output port label"),
                 .direction = std::string("out"),
                 .mediaType = std::string("audio")});

    auto baselineNodeOpt = findNode(scene, 400);
    auto portsNodeOpt = findNode(scene, 401);
    QVERIFY(baselineNodeOpt.has_value());
    QVERIFY(portsNodeOpt.has_value());

    NodeItem &baselineNodeRef = ref(baselineNodeOpt);
    NodeItem &portsNodeRef = ref(portsNodeOpt);

    QVERIFY(portsNodeRef.boundingRect().width()
            > baselineNodeRef.boundingRect().width());
}

QTEST_MAIN(PatchbaySceneInteractionTest)
#include "PatchbaySceneInteractionTest.moc"
