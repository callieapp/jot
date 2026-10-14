pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Finds a note by typing a few letters of its title, an alias or its path, as Ctrl+P
/// does in many editors. Enter opens the one highlighted.
Popup {
    id: root

    function choose(row) {
        const path = Vault.switcher.pathAt(row)
        if (path !== "") {
            Vault.notes.notebook = ""
            Vault.notes.query = ""
            Vault.current = path
        }
        close()
    }

    x: (parent ? parent.width - width : 0) / 2
    y: Theme.titleBarHeight + Theme.space6
    width: Math.min(parent ? parent.width - 2 * Theme.space6 : 0, Theme.searchWidth)
    padding: Theme.space3
    modal: true
    focus: true
    onOpened: {
        input.text = ""
        Vault.switcher.query = ""
        input.forceActiveFocus()
    }

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
        spacing: Theme.space3

        Field {
            id: input

            width: parent.width
            placeholderText: qsTr("Go to a note")
            onTextEdited: {
                Vault.switcher.query = text
                results.currentIndex = 0
            }
            Keys.onDownPressed: results.incrementCurrentIndex()
            Keys.onUpPressed: results.decrementCurrentIndex()
            Keys.onReturnPressed: root.choose(results.currentIndex)
            Keys.onEnterPressed: root.choose(results.currentIndex)
        }

        ListView {
            id: results

            width: parent.width
            height: Math.min(contentHeight, Theme.listRowHeight * 12)
            clip: true
            model: Vault.switcher
            currentIndex: 0

            delegate: AbstractButton {
                id: result

                required property int index
                required property string title
                required property string notebook
                required property string alias

                width: ListView.view.width
                height: Theme.listRowHeight + Theme.space2
                hoverEnabled: true
                Accessible.name: title
                onClicked: root.choose(index)

                background: Rectangle {
                    radius: Theme.radiusSm
                    color: result.ListView.isCurrentItem ? Theme.tint(Theme.accent, 0.18) :
                                                           result.hovered ? Theme.surfaceAlt :
                                                                            Theme.surface
                }

                contentItem: Item {
                    Text {
                        id: name

                        anchors {
                            left: parent.left
                            leftMargin: Theme.space4
                            verticalCenter: parent.verticalCenter
                        }
                        width: Math.min(implicitWidth, parent.width * 0.6)
                        text: result.alias !== "" ? qsTr("%1 (%2)").arg(result.title).arg(
                                                        result.alias) : result.title
                        elide: Text.ElideRight
                        color: Theme.text
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.textBase
                        font.weight: Theme.weightStrong
                    }
                    Text {
                        anchors {
                            left: name.right
                            leftMargin: Theme.space4
                            right: parent.right
                            rightMargin: Theme.space4
                            verticalCenter: parent.verticalCenter
                        }
                        horizontalAlignment: Text.AlignRight
                        text: result.notebook
                        elide: Text.ElideLeft
                        color: Theme.textFaint
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.textSm
                    }
                }
            }
        }

        Text {
            visible: results.count === 0
            leftPadding: Theme.space4
            bottomPadding: Theme.space2
            text: qsTr("No note matches.")
            color: Theme.textFaint
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textMd
        }
    }
}
