import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The start of a linked note, shown while the pointer rests on its link.
Popup {
    id: root

    property string title
    property string text

    padding: Theme.space4
    width: Math.min(Theme.searchWidth, parent ? parent.width - x - Theme.space4 : Theme.searchWidth)
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: Theme.radiusLg
        color: Theme.surface
        border.color: Theme.border

        SoftShadow {
            anchors.fill: parent
            radius: parent.radius
        }
    }

    contentItem: Column {
        spacing: Theme.space3

        Text {
            width: root.availableWidth
            text: root.title
            elide: Text.ElideRight
            color: Theme.text
            font.family: Theme.displayFontFamily
            font.pixelSize: Theme.textXl
            font.weight: Theme.weightBold
        }
        Text {
            width: root.availableWidth
            text: root.text === "" ? qsTr("This note is empty.") : root.text
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            maximumLineCount: 12
            elide: Text.ElideRight
            color: root.text === "" ? Theme.textFaint : Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textMd
            lineHeight: 1.2
        }
    }
}
