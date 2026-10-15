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
                    right: rightControls.left
                    rightMargin: Theme.space4
                    verticalCenter: parent.verticalCenter
                }
                visible: Vault.isOpen
                glyph: "search"
                Accessible.name: qsTr("Go to a note")
                tip: qsTr("Go to a note (Ctrl+P)")
                onClicked: switcher.open()
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
