import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.soundstream

Kirigami.Page {
    id: root
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
        objectName: "patchbayView"
        anchors.fill: parent

        onNodeActivated: (nodeId) => {
            // Future: open effect drawer for nodeId
        }
    }

    PipeWireStatusBanner {
        connected: patchbay.connected
    }

    ZoomToolbox {
        patchbay: patchbay
    }

    MediaLegend {}
}
