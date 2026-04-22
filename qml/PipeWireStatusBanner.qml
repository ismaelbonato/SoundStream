import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.InlineMessage {
    id: root

    property bool connected: false

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
    visible: !connected
    type: Kirigami.MessageType.Warning
    text: i18n("Not connected to PipeWire daemon.")
    z: 10
}
