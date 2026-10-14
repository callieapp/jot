import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The note chosen, as its plain text for now; the Markdown editor takes this place next.
Rectangle {
    id: root

    color: Theme.bg

    Text {
        anchors.centerIn: parent
        visible: Vault.current === ""
        text: Vault.noteCount === 0 ? qsTr("This vault has no notes yet.") : qsTr(
                                          "Choose a note, or press Ctrl+P to find one.")
        color: Theme.textFaint
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textBase
    }

    ScrollView {
        anchors.fill: parent
        visible: Vault.current !== ""
        contentWidth: availableWidth

        Column {
            width: parent.width
            padding: Theme.space7
            spacing: Theme.space5

            Text {
                width: parent.width - 2 * parent.padding
                text: Vault.currentTitle
                wrapMode: Text.WordWrap
                color: Theme.text
                font.family: Theme.displayFontFamily
                font.pixelSize: Theme.text2xl
                font.weight: Theme.weightBold
            }
            TextEdit {
                width: parent.width - 2 * parent.padding
                text: Vault.currentText
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                color: Theme.text
                selectionColor: Theme.tint(Theme.accent, 0.4)
                selectedTextColor: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textBase
            }
        }
    }
}
