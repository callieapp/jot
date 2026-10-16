import Jot.Ui
import QtQuick

/// The X at the top right of a dialog. Whoever places it says what closing does.
StickerButton {
    objectName: "close"
    glyph: "close"
    Accessible.name: qsTr("Close")
}
