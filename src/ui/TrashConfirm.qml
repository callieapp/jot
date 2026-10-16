import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Asks before a note or a notebook goes to the trash, as the editor's Delete does, since a
/// vault's own trash is out of sight.
Popup {
    id: root

    /// What goes: a note's path, or a notebook's when `notebook` is set.
    property string path
    property bool notebook: false

    /// Opens it for the note at `path`.
    function askForNote(path) {
        root.path = path
        root.notebook = false
        open()
    }
    /// Opens it for the notebook at `path`.
    function askForNotebook(path) {
        root.path = path
        root.notebook = true
        open()
    }

    anchors.centerIn: Overlay.overlay
    width: 380
    padding: Theme.space6
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    Overlay.modal: Scrim {}

    background: Rectangle {
        radius: Theme.radiusXl
        color: Theme.surface
        border.color: Theme.border

        SoftShadow {
            anchors.fill: parent
            radius: parent.radius
        }

        // In the corner, clear of what the card says.
        CloseButton {
            anchors {
                top: parent.top
                right: parent.right
                margins: Theme.space4
            }
            onClicked: root.close()
        }
    }

    contentItem: Column {
        spacing: Theme.space4

        Text {
            // Room for the X in the corner.
            width: parent.width - Theme.space7
            wrapMode: Text.Wrap
            text: {
                if (!root.notebook)
                    return qsTr("Move %1 to the trash?").arg(Vault.titleOf(root.path))
                const name = root.path.split("/").pop()
                const count = Vault.notesInNotebook(root.path)
                return count === 0 ? qsTr("Move the notebook %1 to the trash?").arg(name) : qsTr(
                                         "Move the notebook %1 and its %n note(s) to the trash?", "",
                                         count).arg(name)
            }
            color: Theme.text
            font.family: Theme.displayFontFamily
            font.pixelSize: Theme.textLg
            font.weight: Theme.weightBold
        }
        Text {
            width: parent.width
            wrapMode: Text.Wrap
            text: root.notebook ? qsTr(
                                      "It goes to your desktop's trash with everything in it. Where the desktop has none for this folder, it goes to the .jot/trash folder in your notes folder.") :
                                  qsTr("It goes to your desktop's trash, with its attachments. Where the desktop has none for this folder, it goes to the .jot/trash folder in your notes folder.")
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textSm
        }
        Row {
            spacing: Theme.space2

            StickerButton {
                objectName: "confirmTrash"
                text: qsTr("Move to trash")
                destructive: true
                armed: true
                onClicked: {
                    if (root.notebook)
                        Vault.trashNotebook(root.path)
                    else
                        Vault.trashNote(root.path)
                    root.close()
                }
            }
            StickerButton {
                text: qsTr("Cancel")
                onClicked: root.close()
            }
        }
    }
}
