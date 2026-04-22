import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ShadowedRectangle {
    id: root

    anchors {
        top: parent.top
        right: parent.right
        topMargin: Kirigami.Units.largeSpacing
        rightMargin: Kirigami.Units.largeSpacing
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

    implicitWidth: legendColumn.implicitWidth + Kirigami.Units.mediumSpacing * 2
    implicitHeight: legendColumn.implicitHeight + Kirigami.Units.mediumSpacing * 2

    Column {
        id: legendColumn
        anchors.fill: parent
        anchors.margins: Kirigami.Units.mediumSpacing
        spacing: Kirigami.Units.smallSpacing

        Text {
            text: i18n("Media")
            color: Kirigami.Theme.textColor
            font.bold: true
            font.pixelSize: 12
        }

        Repeater {
            model: [
                { label: i18n("Audio"), color: Kirigami.Theme.highlightColor },
                { label: i18n("MIDI"),  color: Kirigami.Theme.linkColor },
                { label: i18n("Video"), color: Kirigami.Theme.neutralTextColor }
            ]

            delegate: Row {
                spacing: Kirigami.Units.smallSpacing

                Rectangle {
                    width: 12
                    height: 12
                    radius: 3
                    color: modelData.color
                    border.color: Kirigami.ColorUtils.tintWithAlpha(
                        modelData.color,
                        Kirigami.Theme.textColor,
                        0.35
                    )
                    border.width: 1
                }

                Text {
                    text: modelData.label
                    color: Kirigami.Theme.textColor
                    font.pixelSize: 11
                }
            }
        }
    }
}
