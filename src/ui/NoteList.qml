pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The middle column: a search box, the order, and the notes of the notebook or tag
/// chosen in the sidebar. Choosing a note shows it.
Rectangle {
    id: root

    /// Puts the cursor in the search box.
    function focusSearch() {
        search.forceActiveFocus()
        search.selectAll()
    }

    color: Theme.bg

    Column {
        id: header

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.space4
        }
        spacing: Theme.space3

        Field {
            id: search

            width: parent.width
            placeholderText: Vault.notes.tag !== "" ? qsTr("Search #%1").arg(Vault.notes.tag) :
                                                      Vault.notes.notebook !== "" ? qsTr(
                                                                                        "Search %1").arg(
                                                                                        Vault.notes.notebook) :
                                                                                    qsTr("Search notes")
            text: Vault.notes.query
            onTextEdited: Vault.notes.query = text
            Keys.onDownPressed: list.forceActiveFocus()
            Keys.onEscapePressed: {
                text = ""
                Vault.notes.query = ""
            }
        }

        Item {
            width: parent.width
            height: order.height

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: Vault.notes.count === 1 ? qsTr("1 note") : qsTr("%1 notes").arg(
                                                    Vault.notes.count)
                color: Theme.textFaint
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
                font.weight: Theme.weightStrong
            }
            Dropdown {
                id: order

                anchors.right: parent.right
                visible: Vault.notes.query === ""
                minimumWidth: 140
                model: [
                    {
                        "id": "updated",
                        "name": qsTr("Recently changed")
                    },
                    {
                        "id": "title",
                        "name": qsTr("By title")
                    }
                ]
                currentValue: Vault.notes.order
                onActivated: value => Vault.notes.order = value
            }
        }
    }

    ListView {
        id: list

        anchors {
            left: parent.left
            right: parent.right
            top: header.bottom
            bottom: parent.bottom
            topMargin: Theme.space3
        }
        clip: true
        model: Vault.notes
        currentIndex: Vault.notes.rowOf(Vault.current)
        keyNavigationEnabled: true
        onCurrentIndexChanged: {
            if (activeFocus && currentIndex >= 0)
                Vault.current = Vault.notes.pathAt(currentIndex)
        }
        ScrollBar.vertical: ScrollBar {}

        delegate: AbstractButton {
            id: note

            required property int index
            required property string path
            required property string title
            required property string notebook
            required property date modified
            required property string snippet
            readonly property bool selected: Vault.current === path

            width: ListView.view.width
            height: card.implicitHeight + Theme.space2
            hoverEnabled: true
            Accessible.name: title

            // A note drags onto a notebook in the sidebar to move there.
            DragHandler {
                id: noteDrag
                target: null
                enabled: Vault.writable
            }
            Drag.active: noteDrag.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.MoveAction
            Drag.mimeData: ({
                                "application/x-jot-note": note.path
                            })
            onClicked: {
                Vault.current = path
                list.currentIndex = index
            }

            background: Rectangle {
                anchors.fill: parent
                anchors.leftMargin: Theme.space3
                anchors.rightMargin: Theme.space3
                radius: Theme.radiusMd
                color: note.selected ? Theme.tint(Theme.accent, 0.18) : note.hovered
                                       ? Theme.surfaceAlt : Theme.bg
            }

            contentItem: Column {
                id: card

                leftPadding: Theme.space5
                rightPadding: Theme.space5
                topPadding: Theme.space3
                bottomPadding: Theme.space3
                spacing: Theme.space1

                Text {
                    width: note.width - card.leftPadding - card.rightPadding
                    text: note.title
                    elide: Text.ElideRight
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textBase
                    font.weight: note.selected ? Theme.weightBold : Theme.weightStrong
                }
                Text {
                    width: note.width - card.leftPadding - card.rightPadding
                    visible: note.snippet !== ""
                    text: note.snippet.replace(/<mark>/g, "<b>").replace(/<\/mark>/g, "</b>")
                    textFormat: Text.StyledText
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textSm
                }
                Text {
                    width: note.width - card.leftPadding - card.rightPadding
                    text: (note.notebook !== "" && note.notebook !== Vault.notes.notebook
                           ? note.notebook + ", " : "") + note.modified.toLocaleDateString(Qt.locale(
                                                                                               ), Locale.ShortFormat)
                    elide: Text.ElideRight
                    color: Theme.textFaint
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textXs
                }
            }
        }

        Text {
            anchors.centerIn: parent
            width: parent.width - 2 * Theme.space6
            visible: list.count === 0
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: Vault.notes.query !== "" ? qsTr("Nothing matches.") : qsTr("No notes here yet.")
            color: Theme.textFaint
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textMd
        }
    }

    Rectangle {
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        width: 1
        color: Theme.hairline
    }
}
