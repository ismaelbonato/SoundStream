import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ShadowedRectangle {
    id: root

    property var patchbay

    anchors {
        horizontalCenter: parent.horizontalCenter
        bottom: parent.bottom
        bottomMargin: Kirigami.Units.largeSpacing
    }
    z: 20
    radius: 10
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    color: Kirigami.Theme.backgroundColor
    border {
        width: 1
        color: Kirigami.ColorUtils.tintWithAlpha(
            Kirigami.Theme.backgroundColor,
            Kirigami.Theme.textColor,
            0.2
        )
    }
    shadow {
        size: 15
        color: Qt.rgba(0, 0, 0, 0.2)
    }

    implicitWidth: zoomToolBar.implicitWidth + Kirigami.Units.mediumSpacing * 2
    implicitHeight: zoomToolBar.implicitHeight + Kirigami.Units.mediumSpacing * 2

    Kirigami.ActionToolBar {
        id: zoomToolBar
        anchors.centerIn: parent
        flat: true
        position: QQC2.ToolBar.Footer
        actions: [
            Kirigami.Action {
                icon.name: "zoom-out"
                text: i18n("Zoom Out")
                tooltip: text
                enabled: root.patchbay.zoom > root.patchbay.minimumZoom
                displayHint: Kirigami.DisplayHint.IconOnly
                             | Kirigami.DisplayHint.KeepVisible
                onTriggered: root.patchbay.zoomOut()
            },
            Kirigami.Action {
                text: i18n("Zoom")
                displayHint: Kirigami.DisplayHint.KeepVisible
                displayComponent: QQC2.Slider {
                    from: root.patchbay.minimumZoom
                    to: root.patchbay.maximumZoom
                    value: root.patchbay.zoom
                    live: true
                    Layout.preferredWidth: 180
                    onMoved: root.patchbay.zoom = value
                    QQC2.ToolTip.text: i18n("%1%", Math.round(value * 100))
                    QQC2.ToolTip.visible: pressed
                }
            },
            Kirigami.Action {
                text: i18n("Zoom Level")
                displayHint: Kirigami.DisplayHint.KeepVisible
                displayComponent: Text {
                    width: 44
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    text: i18n("%1%", Math.round(root.patchbay.zoom * 100))
                    color: Kirigami.Theme.textColor
                    font.pixelSize: 12
                }
            },
            Kirigami.Action {
                icon.name: "zoom-in"
                text: i18n("Zoom In")
                tooltip: text
                enabled: root.patchbay.zoom < root.patchbay.maximumZoom
                displayHint: Kirigami.DisplayHint.IconOnly
                             | Kirigami.DisplayHint.KeepVisible
                onTriggered: root.patchbay.zoomIn()
            },
            Kirigami.Action {
                icon.name: "zoom-fit-best"
                text: i18n("Center")
                tooltip: text
                displayHint: Kirigami.DisplayHint.IconOnly
                             | Kirigami.DisplayHint.KeepVisible
                onTriggered: root.patchbay.centerView()
            }
        ]
    }
}
