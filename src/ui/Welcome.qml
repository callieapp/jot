import Jot.Ui
import QtQuick
import QtQuick.Dialogs

/// What Jot shows before a vault is open: the logo, a word on what a vault is, and a
/// button to choose one.
Item {
    id: root

    FolderDialog {
        id: chooser
        title: qsTr("Choose a folder for your notes")
        onAccepted: Vault.openFolder(selectedFolder)
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - 2 * Theme.space7, 420)
        spacing: Theme.space5

        Logo {
            anchors.horizontalCenter: parent.horizontalCenter
            size: 96
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Welcome to Jot")
            color: Theme.text
            font.family: Theme.displayFontFamily
            font.pixelSize: Theme.text2xl
            font.weight: Theme.weightBold
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: qsTr("Your notes live in a folder you choose, as plain Markdown files. Pick an "
                       + "empty folder to start, or one that already holds notes.")
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textBase
            lineHeight: 1.3
        }
        StickerButton {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Choose a folder")
            accent: true
            onClicked: chooser.open()
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            visible: Vault.error !== ""
            text: Vault.error
            color: Theme.danger
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textMd
        }
    }
}
