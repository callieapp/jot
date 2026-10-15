pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Every keyboard shortcut, from the list the window's shortcuts are made from.
Popup {
    id: root

    /// {keys, label, inNote} for each shortcut, `keys` being one or more ways to type it
    /// and `inNote` true for those that work only while writing.
    required property var actions

    anchors.centerIn: Overlay.overlay
    width: 620
    height: Math.min(implicitHeight, (parent ? parent.height : 800) - 2 * Theme.space6)
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
    }

    contentItem: Flickable {
        implicitHeight: content.implicitHeight
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: content
            width: parent.width
            spacing: Theme.space4

            Text {
                text: qsTr("Keyboard shortcuts")
                color: Theme.text
                font.family: Theme.displayFontFamily
                font.pixelSize: Theme.textXl
                font.weight: Theme.weightBold
            }

            Table {
                title: qsTr("Anywhere")
                rows: root.actions.filter(a => !a.inNote)
            }
            Table {
                title: qsTr("While writing")
                rows: root.actions.filter(a => a.inNote)
            }
        }
    }

    /// A titled list of keys and what they do, in two columns.
    component Table: Column {
        id: table

        required property string title
        required property var rows

        width: parent.width
        spacing: Theme.space2

        Text {
            text: table.title
            color: Theme.textFaint
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textSm
            font.weight: Theme.weightHeavy
        }
        Grid {
            columns: 2
            columnSpacing: Theme.space5
            rowSpacing: Theme.space2

            Repeater {
                model: table.rows

                Row {
                    id: row
                    required property var modelData
                    width: (table.width - Theme.space5) / 2
                    spacing: Theme.space3

                    Row {
                        id: caps
                        spacing: Theme.space1

                        Repeater {
                            model: row.modelData.keys

                            KeyCap {
                                required property string modelData
                                keys: modelData
                            }
                        }
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        width: row.width - caps.width - row.spacing
                        elide: Text.ElideRight
                        text: row.modelData.label
                        color: Theme.text
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.textSm
                    }
                }
            }
        }
    }
}
