// Main.qml – Kirigami ApplicationWindow
import QtQuick
import org.kde.kirigami as Kirigami

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

    // ── Page ──────────────────────────────────────────────────────────────
    pageStack.initialPage: PatchbayPage {}
}
