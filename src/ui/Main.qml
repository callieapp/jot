pragma ComponentBehavior: Bound
import Jot.Ui

import QtQuick
import QtQuick.Controls
import QtQuick.Effects

/// The main window: a frameless frame with Jot's own title bar, as Callie draws it.
ApplicationWindow {
    id: window

    width: 1280
    height: 840
    minimumWidth: 600
    minimumHeight: 560
    visible: true
    title: qsTr("Jot")
    // Transparent where the rounded frame leaves its corners.
    color: frame.rounded ? "transparent" : Theme.bg
    flags: Qt.Window | Qt.FramelessWindowHint

    // Everything the window shows, with its corners rounded unless it fills
    // the screen.
    Item {
        id: frame

        // The software renderer has no shader effects, so it keeps square corners.
        readonly property bool rounded: window.visibility === Window.Windowed && GraphicsInfo.api
                                        !== GraphicsInfo.Software

        anchors.fill: parent
        layer.enabled: rounded
        layer.effect: MultiEffect {
            maskEnabled: true
            maskSource: frameMask
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1
        }

        Rectangle {
            anchors.fill: parent
            color: Theme.bg
        }

        Rectangle {
            id: titleBar

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: Theme.titleBarHeight
            color: Theme.surface

            // Behind the controls: dragging the bar moves the window, double-clicking
            // maximizes it, as a desktop title bar would.
            DragHandler {
                target: null
                onActiveChanged: {
                    if (active)
                        window.startSystemMove()
                }
            }
            TapHandler {
                onDoubleTapped: window.visibility === Window.Maximized ? window.showNormal() :
                                                                         window.showMaximized()
            }

            Rectangle {
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                height: 1
                color: Theme.border
            }

            Row {
                id: leading

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: WindowButtons.left.length > 0 ? Theme.space4 : Theme.space5
                }
                spacing: Theme.space4

                WindowControls {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: buttons.length > 0
                    buttons: WindowButtons.left
                }
                Logo {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -Theme.logoLift
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: window.title
                    color: Theme.text
                    font.family: Theme.displayFontFamily
                    font.pixelSize: Theme.textDisplay
                    font.weight: Theme.weightBold
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: Vault.isOpen
                    text: Vault.name
                    color: Theme.textMuted
                    font.family: Theme.displayFontFamily
                    font.pixelSize: Theme.textDisplay
                    font.weight: Theme.weightStrong
                }
            }

            StickerButton {
                id: newButton

                anchors {
                    right: searchButton.left
                    rightMargin: Theme.space3
                    verticalCenter: parent.verticalCenter
                }
                visible: Vault.isOpen && Vault.writable
                glyph: "plus"
                Accessible.name: qsTr("New note")
                tip: qsTr("New note (Ctrl+N)")
                onClicked: {
                    if (Vault.newNote() !== "")
                        editor.editTitle()
                }
            }

            StickerButton {
                id: searchButton

                anchors {
                    right: settingsButton.left
                    rightMargin: Theme.space3
                    verticalCenter: parent.verticalCenter
                }
                visible: Vault.isOpen
                glyph: "search"
                Accessible.name: qsTr("Go to a note")
                tip: qsTr("Go to a note (Ctrl+P)")
                onClicked: switcher.open()
            }

            StickerButton {
                id: settingsButton

                anchors {
                    right: helpButton.left
                    rightMargin: Theme.space3
                    verticalCenter: parent.verticalCenter
                }
                glyph: "settings"
                glyphStroke: Theme.fineGlyphStroke
                Accessible.name: qsTr("Settings")
                tip: qsTr("Settings (Ctrl+,)")
                onClicked: settingsDialog.open()
            }

            // Help: debug info, logs and bug reports.
            StickerButton {
                id: helpButton

                anchors {
                    right: rightControls.left
                    rightMargin: Theme.space4
                    verticalCenter: parent.verticalCenter
                }
                text: "?"
                Accessible.name: qsTr("Help")
                // Not beside its own open menu, which leaves the button hovered.
                tip: helpMenu.visible ? "" : qsTr("Help, shortcuts and about")
                onClicked: helpMenu.popup(helpButton, 0, helpButton.height + Theme.space3)

                Tip {
                    visible: copiedTip.running
                    text: qsTr("Debug info copied")
                }

                // A newer release is out; the menu says which. It sits on the corner,
                // a fifth of it past the edge, ringed in the background color.
                Rectangle {
                    visible: Release.update !== ""
                    width: Theme.badgeSize
                    height: Theme.badgeSize
                    x: parent.width - width * 0.8
                    y: -height * 0.2
                    radius: width / 2
                    color: Theme.accent
                    border.width: width / 5
                    border.color: Theme.bg
                }

                Timer {
                    id: copiedTip
                    interval: 2000
                }

                Connections {
                    target: Support
                    function onCopied() {
                        copiedTip.restart()
                    }
                }

                MenuCard {
                    id: helpMenu
                    minimumWidth: 200

                    MenuEntry {
                        text: qsTr("Update to Jot %1...").arg(Release.update)
                        visible: Release.update !== ""
                        height: visible ? implicitHeight : 0
                        onTriggered: updateDialog.open()
                    }
                    MenuEntry {
                        text: qsTr("Copy debug info")
                        onTriggered: Support.copyDebugInfo()
                    }
                    MenuEntry {
                        text: qsTr("Open logs folder")
                        onTriggered: Support.openLogs()
                    }
                    MenuEntry {
                        text: qsTr("Report a bug...")
                        onTriggered: Support.reportBug()
                    }
                    MenuSeparator {
                        contentItem: Rectangle {
                            implicitHeight: 1
                            color: Theme.border
                        }
                    }
                    MenuEntry {
                        text: qsTr("Keyboard shortcuts")
                        onTriggered: keyHelp.open()
                    }
                    MenuEntry {
                        text: qsTr("What's new")
                        visible: Release.notes !== ""
                        height: visible ? implicitHeight : 0
                        onTriggered: whatsNew.open()
                    }
                    MenuEntry {
                        text: qsTr("About Jot")
                        onTriggered: about.open()
                    }
                    MenuEntry {
                        text: qsTr("Support Jot on Ko-fi")
                        onTriggered: Qt.openUrlExternally(Release.support)
                    }
                }
            }

            WindowControls {
                id: rightControls

                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: Theme.space4
                }
                visible: buttons.length > 0
                buttons: WindowButtons.right
            }
        }

        Item {
            anchors {
                left: parent.left
                right: parent.right
                top: titleBar.bottom
                bottom: parent.bottom
            }

            Welcome {
                anchors.fill: parent
                visible: !Vault.isOpen
            }

            Sidebar {
                id: sidebar

                visible: Vault.isOpen
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: parent.bottom
                }
                width: Theme.sidebarWidth
            }
            TodoList {
                visible: Vault.isOpen && Vault.showingTodos
                anchors {
                    left: sidebar.right
                    top: parent.top
                    bottom: parent.bottom
                }
                width: Theme.noteListWidth
            }
            NoteList {
                id: noteList

                visible: Vault.isOpen && !Vault.showingTodos
                anchors {
                    left: sidebar.right
                    top: parent.top
                    bottom: parent.bottom
                }
                width: Theme.noteListWidth
            }
            NoteEditor {
                id: editor

                visible: Vault.isOpen
                anchors {
                    left: noteList.right
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                }
            }
        }
    }

    QuickSwitcher {
        id: switcher
    }

    LazyPopup {
        id: settingsDialog
        sourceComponent: SettingsDialog {}
    }
    LazyPopup {
        id: keyHelp
        sourceComponent: KeyHelp {
            actions: window.shortcuts
        }
    }
    LazyPopup {
        id: about
        sourceComponent: AboutDialog {
            onWhatsNewRequested: whatsNew.open()
        }
    }
    LazyPopup {
        id: whatsNew
        sourceComponent: WhatsNewDialog {}
    }
    LazyPopup {
        id: updateDialog
        sourceComponent: UpdateDialog {}
    }

    // After an update, says what changed, once the window has settled.
    Timer {
        interval: 400
        running: true
        onTriggered: {
            if (Release.takeUpdateNotice())
                whatsNew.open()
        }
    }

    /// What the keyboard shortcuts dialog lists.
    readonly property var shortcuts: [
        {
            "keys": ["Ctrl+N"],
            "label": qsTr("New note")
        },
        {
            "keys": ["Ctrl+P"],
            "label": qsTr("Go to a note")
        },
        {
            "keys": ["Ctrl+F"],
            "label": qsTr("Search notes")
        },
        {
            "keys": ["Ctrl+,"],
            "label": qsTr("Settings")
        },
        {
            "keys": ["F1"],
            "label": qsTr("Keyboard shortcuts")
        },
        {
            "keys": ["Ctrl+S"],
            "label": qsTr("Save now"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+B"],
            "label": qsTr("Bold"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+I"],
            "label": qsTr("Italic"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+Shift+X"],
            "label": qsTr("Strikethrough"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+E"],
            "label": qsTr("Code"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+K"],
            "label": qsTr("Link"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+1", "Ctrl+2", "Ctrl+3"],
            "label": qsTr("Headings"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+Enter"],
            "label": qsTr("Checkbox"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+Click"],
            "label": qsTr("Follow a link"),
            "inNote": true
        },
        {
            "keys": ["Ctrl+V"],
            "label": qsTr("Paste an image"),
            "inNote": true
        }
    ]

    Shortcut {
        sequences: [StandardKey.New]
        enabled: Vault.isOpen && Vault.writable
        onActivated: {
            if (Vault.newNote() !== "")
                editor.editTitle()
        }
    }
    Shortcut {
        sequences: [StandardKey.Save]
        enabled: Vault.isOpen
        onActivated: Vault.saveNow()
    }
    Shortcut {
        sequence: "Ctrl+P"
        enabled: Vault.isOpen
        onActivated: switcher.open()
    }
    Shortcut {
        sequences: [StandardKey.Preferences, "Ctrl+,"]
        onActivated: settingsDialog.open()
    }
    Shortcut {
        sequences: [StandardKey.HelpContents]
        onActivated: keyHelp.open()
    }
    Shortcut {
        sequences: [StandardKey.Find]
        enabled: Vault.isOpen
        onActivated: noteList.focusSearch()
    }

    Rectangle {
        id: frameMask
        anchors.fill: frame
        radius: Theme.radiusMd
        visible: false
        layer.enabled: true
    }

    ResizeFrame {
        anchors.fill: parent
        z: 1000
    }
}
