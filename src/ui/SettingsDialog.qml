pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

/// Jot's preferences. Every change applies and saves at once, so there is
/// nothing to confirm; reset puts everything back.
Popup {
    id: root

    anchors.centerIn: Overlay.overlay
    width: Math.min(Overlay.overlay ? Overlay.overlay.width - 2 * Theme.space6 : 560, 560)
    padding: 0
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    /// The user wants to edit the theme's colors, which needs the window in view.
    signal editColorsRequested

    /// Which group of settings shows: "general", "appearance" or "developer".
    property string tab: "general"

    Overlay.modal: Scrim {}

    enter: Transition {
        NumberAnimation {
            property: "scale"
            from: 0.94
            to: 1
            duration: Theme.durMed
            easing.type: Theme.easingBounce
            easing.overshoot: Theme.bounce
        }
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: Theme.durFast
        }
    }

    background: Rectangle {
        radius: Theme.radiusXl
        color: Theme.surface
        border.color: Theme.border

        SoftShadow {
            anchors.fill: parent
            radius: parent.radius
        }
    }

    contentItem: Column {
        padding: Theme.space6
        spacing: Theme.space5

        Item {
            width: parent.width - 2 * parent.padding
            height: title.implicitHeight

            Text {
                id: title
                text: qsTr("Settings")
                color: Theme.text
                font.family: Theme.displayFontFamily
                font.pixelSize: Theme.textXl
                font.weight: Theme.weightBold
            }

            StickerButton {
                anchors {
                    right: parent.right
                    verticalCenter: title.verticalCenter
                }
                glyph: "close"
                Accessible.name: qsTr("Close")
                onClicked: root.close()
            }
        }

        Flow {
            width: root.width - 2 * Theme.space6
            spacing: Theme.space2

            Repeater {
                model: [
                    {
                        "id": "general",
                        "label": qsTr("General")
                    },
                    {
                        "id": "appearance",
                        "label": qsTr("Theme")
                    },
                    {
                        "id": "developer",
                        "label": qsTr("Developer")
                    }
                ]

                PillButton {
                    id: tabButton
                    required property var modelData
                    label: tabButton.modelData.label
                    selected: root.tab === tabButton.modelData.id
                    onClicked: root.tab = tabButton.modelData.id
                }
            }
        }

        Section {
            title: qsTr("Notes folder")
            visible: root.tab === "general"

            Text {
                width: parent.width
                text: Vault.isOpen ? Vault.root : qsTr("No folder chosen yet")
                textFormat: Text.PlainText
                elide: Text.ElideMiddle
                color: Vault.isOpen ? Theme.text : Theme.textMuted
                font.family: Theme.monoFontFamily
                font.pixelSize: Theme.textSm
            }
            Text {
                visible: Vault.isOpen && !Vault.writable
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Jot can read this folder but not change it.")
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }
            Flow {
                width: parent.width
                spacing: Theme.space2

                StickerButton {
                    text: qsTr("Change folder...")
                    onClicked: folderDialog.open()
                }
                StickerButton {
                    visible: Vault.isOpen
                    text: qsTr("Show in Files")
                    onClicked: Support.openFolder("vault")
                }
            }
            Text {
                visible: Vault.error !== ""
                width: parent.width
                text: Vault.error
                wrapMode: Text.Wrap
                color: Theme.danger
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }
        }

        Section {
            title: qsTr("Updates")
            visible: root.tab === "general" && !Release.devel

            Toggle {
                width: parent.width
                text: qsTr("Say when a new release is out")
                checked: Settings.checkForUpdates
                onToggled: Settings.checkForUpdates = checked
            }
            Text {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr(
                          "Once a day, Jot asks GitHub for its latest release. The app shows it in the ? menu, and the jot command after it runs.")
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }
        }

        Section {
            title: qsTr("Theme")
            visible: root.tab === "appearance"

            Dropdown {
                model: Themes.available
                currentValue: Themes.current
                Accessible.name: qsTr("Theme")
                onActivated: value => Themes.use(value)
            }

            Flow {
                width: parent.width
                spacing: Theme.space2

                StickerButton {
                    text: Themes.editable ? qsTr("Edit colors") : qsTr("Customize colors")
                    onClicked: {
                        if (Themes.customize())
                            root.editColorsRequested()
                    }
                }
                StickerButton {
                    text: qsTr("Import...")
                    onClicked: importDialog.open()
                }
                StickerButton {
                    text: qsTr("Export...")
                    onClicked: exportDialog.open()
                }
                // Deleting takes a second click, since the file is gone for good.
                DangerButton {
                    visible: Themes.editable
                    label: qsTr("Delete theme")
                    armedLabel: qsTr("Click again to delete")
                    onConfirmed: Themes.removeCurrent()
                }
            }

            Text {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Themes are shared with Callie, so a theme file works in either app.")
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }

            Text {
                visible: Themes.error !== ""
                width: parent.width
                text: Themes.error
                wrapMode: Text.Wrap
                color: Theme.danger
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }
        }

        Section {
            title: qsTr("Folders")
            visible: root.tab === "developer"

            Flow {
                width: parent.width
                spacing: Theme.space2

                StickerButton {
                    text: qsTr("Settings")
                    onClicked: Support.openFolder("config")
                }
                StickerButton {
                    text: qsTr("Search index")
                    onClicked: Support.openFolder("cache")
                }
                StickerButton {
                    text: qsTr("Logs")
                    onClicked: Support.openLogs()
                }
            }
            Text {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr(
                          "Your notes are only ever the files in your notes folder. The search index is rebuilt from them whenever it is missing.")
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }
        }

        DangerButton {
            visible: root.tab === "general"
            label: qsTr("Reset to defaults")
            armedLabel: qsTr("Click again to reset")
            // Settings forget the theme too, so the default one comes back.
            onConfirmed: {
                Settings.reset()
                Themes.useDefault()
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Choose a folder for your notes")
        currentFolder: Vault.rootUrl
        onAccepted: Vault.openFolder(selectedFolder)
    }

    FileDialog {
        id: importDialog
        title: qsTr("Import a theme")
        nameFilters: [qsTr("Themes (*.toml *.yaml *.yml)")]
        onAccepted: Themes.importFrom(selectedFile)
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export this theme")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "toml"
        nameFilters: [qsTr("Themes (*.toml)")]
        onAccepted: Themes.exportTo(selectedFile)
    }

    /// A small heading over a group of related settings.
    component Section: Column {
        id: section

        property string title
        default property alias content: rows.data

        width: root.width - 2 * Theme.space6
        spacing: Theme.space3

        Text {
            text: section.title.toUpperCase()
            color: Theme.textFaint
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textXs
            font.weight: Theme.weightHeavy
            font.letterSpacing: 0.6
        }
        Column {
            id: rows
            width: parent.width
            spacing: Theme.space3
        }
    }
}
