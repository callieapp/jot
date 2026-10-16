pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Every keyboard shortcut, from the table the window's shortcuts are made from, in tabs:
/// those that work anywhere, those for writing, and vi mode's.
Popup {
    id: root

    /// KeyRouter's actions: {label, scope, keys, vi, leader} for each.
    required property var actions
    /// "general", "editor" or "vi".
    property string tab: "general"

    /// The actions of `scope` that have keys of `kind` ("keys", "vi" or "leader"), as
    /// {keys, label} rows.
    function rows(scope, kind) {
        const found = []
        for (const action of actions) {
            if ((scope === "" || action.scope === scope) && action[kind].length > 0)
                found.push({
                               "keys": kind === "keys" ? action.keys : [action[kind]],
                               "label": action.label
                           })
        }
        return found
    }

    anchors.centerIn: Overlay.overlay
    width: 620
    // Steady across tabs. The overlay, not the parent: in a LazyPopup the parent is a
    // loader with no size.
    height: Math.min(640, (Overlay.overlay ? Overlay.overlay.height : 800) - 2 * Theme.space6)
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

    contentItem: Column {
        spacing: Theme.space4

        Item {
            width: parent.width
            height: title.implicitHeight

            Text {
                id: title
                text: qsTr("Keyboard shortcuts")
                color: Theme.text
                font.family: Theme.displayFontFamily
                font.pixelSize: Theme.textXl
                font.weight: Theme.weightBold
            }
            CloseButton {
                anchors {
                    right: parent.right
                    verticalCenter: title.verticalCenter
                }
                onClicked: root.close()
            }
        }

        Row {
            spacing: Theme.space2

            Repeater {
                model: [
                    {
                        "id": "general",
                        "label": qsTr("General")
                    },
                    {
                        "id": "editor",
                        "label": qsTr("Editor")
                    },
                    {
                        "id": "vi",
                        "label": qsTr("Vi mode")
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

        Flickable {
            width: parent.width
            height: root.availableHeight - y
            contentHeight: content.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            Column {
                id: content
                width: parent.width
                spacing: Theme.space4

                Table {
                    visible: root.tab === "general"
                    title: qsTr("Anywhere")
                    rows: root.rows("window", "keys").concat(root.rows("list", "keys"))
                }

                Table {
                    visible: root.tab === "editor"
                    title: qsTr("While writing")
                    rows: root.rows("note", "keys")
                }

                Text {
                    visible: root.tab === "vi"
                    width: parent.width
                    wrapMode: Text.Wrap
                    text: Settings.viMode ? qsTr(
                                                "Vi mode is on. A note opens in normal mode, where keys move and edit; i or a starts typing and Esc stops.") :
                                            qsTr("Vi mode is off; turn it on under Settings, Keyboard.")
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textSm
                }
                Table {
                    visible: root.tab === "vi"
                    title: qsTr("Out of the note")
                    rows: root.rows("window", "vi")
                }
                Table {
                    visible: root.tab === "vi"
                    title: qsTr("In the note, normal mode")
                    rows: root.rows("note", "vi")
                }
                Table {
                    visible: root.tab === "vi"
                    title: qsTr("In the note, visual mode")
                    rows: root.rows("visual", "vi").concat([
                                                               {
                                                                   "keys": ["h", "j", "k", "l"],
                                                                   "label": qsTr(
                                                                                "Select, by normal mode's moves")
                                                               }
                                                           ])
                }
                Table {
                    visible: root.tab === "vi"
                    title: qsTr("After the leader (%1)").arg(Settings.leaderKey === " " ? qsTr(
                                                                                              "Space") :
                                                                                          Settings.leaderKey)
                    rows: root.rows("", "leader")
                }
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
