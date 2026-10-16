pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The middle column when Tasks is chosen: notes that are tasks, then every unticked
/// checkbox in the vault. Ticking one changes its note; choosing one opens it there.
Rectangle {
    id: root

    color: Theme.bg

    Text {
        id: header

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.space5
        }
        text: Vault.tasks.count === 0 ? qsTr("Nothing left to do.") : Vault.tasks.count === 1 ? qsTr(
                                                                                                    "1 thing to do") :
                                                                                                qsTr("%1 things to do").arg(
                                                                                                    Vault.tasks.count)
        color: Theme.textMuted
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textMd
        font.weight: Theme.weightStrong
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
        model: Vault.tasks
        spacing: Theme.space1
        ScrollBar.vertical: ScrollBar {}

        section.property: "group"
        section.criteria: ViewSection.FullString
        section.delegate: Text {
            required property string section

            width: ListView.view.width
            leftPadding: Theme.space5
            topPadding: Theme.space4
            bottomPadding: Theme.space1
            text: section
            elide: Text.ElideRight
            color: Theme.textFaint
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textXs
            font.weight: Theme.weightBold
            font.capitalization: Font.AllUppercase
        }

        delegate: AbstractButton {
            id: item

            required property string kind
            required property string path
            required property string title
            required property string task
            required property int line
            required property string due
            required property bool overdue

            width: ListView.view.width
            height: Math.max(Theme.listRowHeight, words.implicitHeight + 2 * Theme.space2)
            hoverEnabled: true
            Accessible.name: kind === "note" ? title : task
            onClicked: Vault.openAt(path, line)

            background: Rectangle {
                anchors.fill: parent
                anchors.leftMargin: Theme.space3
                anchors.rightMargin: Theme.space3
                radius: Theme.radiusSm
                color: item.hovered ? Theme.surfaceAlt : Theme.bg
            }

            contentItem: Item {
                AbstractButton {
                    id: box

                    x: Theme.space5
                    anchors.verticalCenter: parent.verticalCenter
                    width: Theme.textBase + Theme.space1
                    height: width
                    enabled: Vault.writable
                    Accessible.name: qsTr("Done")
                    Accessible.role: Accessible.CheckBox
                    onClicked: item.kind === "note" ? Vault.completeTask(item.path) : Vault.setTaskDone(
                                                          item.path, item.line, true)

                    background: Rectangle {
                        radius: Theme.radiusSm
                        color: box.hovered ? Theme.tint(Theme.accent, 0.25) : Theme.surface
                        border.color: box.hovered ? Theme.accent : Theme.border
                        border.width: 2
                    }
                }
                Text {
                    id: words

                    anchors {
                        left: box.right
                        leftMargin: Theme.space4
                        right: dueLabel.left
                        rightMargin: Theme.space3
                        verticalCenter: parent.verticalCenter
                    }
                    text: item.kind === "note" ? item.title : item.task
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textBase
                    font.weight: item.kind === "note" ? Theme.weightBold : Theme.weightStrong
                }
                Text {
                    id: dueLabel

                    anchors {
                        right: parent.right
                        rightMargin: Theme.space5
                        verticalCenter: parent.verticalCenter
                    }
                    visible: item.due !== ""
                    text: item.due.slice(0, 10)
                    color: item.overdue ? Theme.danger : Theme.textFaint
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textSm
                    font.weight: item.overdue ? Theme.weightBold : Theme.weightStrong
                }
            }
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
