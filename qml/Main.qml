// Main.qml – Kirigami ApplicationWindow
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.soundstream

Kirigami.ApplicationWindow {
    id: root
    title: i18n("SoundStream — PipeWire Patchbay")
    width:  1280
    height: 800
    minimumWidth:  800
    minimumHeight: 500

    // ── Keyboard shortcuts ────────────────────────────────────────────────
    Shortcut {
        sequence: "Ctrl+0"
        onActivated: patchbay.resetView()
    }

    // ── Status banner — driven by PatchbayView.connected ─────────────────
    Kirigami.InlineMessage {
        id: disconnectBanner
        anchors { top: parent.top; left: parent.left; right: parent.right }
        visible: !patchbay.connected
        type: Kirigami.MessageType.Warning
        text: i18n("Not connected to PipeWire daemon.")
        z: 10
    }

    // ── Page ──────────────────────────────────────────────────────────────
    pageStack.initialPage: Kirigami.Page {
        title: i18n("Patchbay")
        padding: 0

        actions: [
            Kirigami.Action {
                icon.name: "view-refresh"
                text: i18n("Refresh Layout")
                onTriggered: patchbay.resetLayout()
            },
            Kirigami.Action {
                icon.name: "zoom-fit-best"
                text: i18n("Reset View")
                onTriggered: patchbay.resetView()
            }
        ]

        PatchbayView {
            id: patchbay
            objectName: "patchbayView"   // used by main.cpp findChild
            anchors.fill: parent

            onNodeActivated: (nodeId) => {
                // Future: open effect drawer for nodeId
            }
        }

        Rectangle {
            id: zoomToolbox
            anchors {
                left: parent.left
                bottom: parent.bottom
                leftMargin: 12
                bottomMargin: 12
            }
            z: 20
            radius: 8
            color: "#AA1B1E22"
            border.color: "#55FFFFFF"
            border.width: 1

            implicitWidth: zoomToolBar.implicitWidth + 16
            implicitHeight: zoomToolBar.implicitHeight + 14

            Kirigami.ActionToolBar {
                id: zoomToolBar
                anchors.centerIn: parent
                flat: true
                actions: [
                    Kirigami.Action {
                        icon.name: "zoom-out"
                        text: i18n("Zoom Out")
                        tooltip: text
                        enabled: patchbay.zoom > patchbay.minimumZoom
                        displayHint: Kirigami.DisplayHint.IconOnly
                                     | Kirigami.DisplayHint.KeepVisible
                        onTriggered: patchbay.zoomOut()
                    },
                    Kirigami.Action {
                        text: i18n("Zoom")
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        displayComponent: QQC2.Slider {
                            from: patchbay.minimumZoom
                            to: patchbay.maximumZoom
                            value: patchbay.zoom
                            live: true
                            Layout.preferredWidth: 180
                            onMoved: patchbay.zoom = value
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
                            text: i18n("%1%", Math.round(patchbay.zoom * 100))
                            color: "#E8EDF2"
                            font.pixelSize: 12
                        }
                    },
                    Kirigami.Action {
                        icon.name: "zoom-in"
                        text: i18n("Zoom In")
                        tooltip: text
                        enabled: patchbay.zoom < patchbay.maximumZoom
                        displayHint: Kirigami.DisplayHint.IconOnly
                                     | Kirigami.DisplayHint.KeepVisible
                        onTriggered: patchbay.zoomIn()
                    },
                    Kirigami.Action {
                        icon.name: "zoom-fit-best"
                        text: i18n("Center")
                        tooltip: text
                        displayHint: Kirigami.DisplayHint.IconOnly
                                     | Kirigami.DisplayHint.KeepVisible
                        onTriggered: patchbay.centerView()
                    }
                ]
            }
        }

        Rectangle {
            id: mediaLegend
            anchors {
                top: parent.top
                right: parent.right
                topMargin: 12
                rightMargin: 12
            }
            z: 20
            radius: 8
            color: "#AA1B1E22"
            border.color: "#55FFFFFF"
            border.width: 1

            implicitWidth: legendColumn.implicitWidth + 16
            implicitHeight: legendColumn.implicitHeight + 14

            Column {
                id: legendColumn
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                Text {
                    text: i18n("Media")
                    color: "#F0F4F8"
                    font.bold: true
                    font.pixelSize: 12
                }

                Repeater {
                    model: [
                        { label: i18n("Audio"), color: "#3A9AFF" },
                        { label: i18n("MIDI"),  color: "#AF5DFF" },
                        { label: i18n("Video"), color: "#FF9F43" }
                    ]

                    delegate: Row {
                        spacing: 6

                        Rectangle {
                            width: 12
                            height: 12
                            radius: 3
                            color: modelData.color
                            border.color: "#AAFFFFFF"
                            border.width: 1
                        }

                        Text {
                            text: modelData.label
                            color: "#E8EDF2"
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }
}
