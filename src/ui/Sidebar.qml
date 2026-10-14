pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The notebooks, as a tree that folds, and the tags below them. Choosing either fills
/// the note list.
Rectangle {
    id: root

    color: Theme.surface

    component Heading: Text {
        leftPadding: Theme.space5
        topPadding: Theme.space5
        bottomPadding: Theme.space2
        color: Theme.textFaint
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textXs
        font.weight: Theme.weightBold
        font.capitalization: Font.AllUppercase
        font.letterSpacing: 0.6
    }

    component Entry: AbstractButton {
        id: row

        property bool selected: false
        property string label

        height: Theme.listRowHeight
        hoverEnabled: true
        Accessible.name: label

        background: Rectangle {
            anchors.fill: parent
            anchors.leftMargin: Theme.space3
            anchors.rightMargin: Theme.space3
            radius: Theme.radiusSm
            color: row.selected ? Theme.tint(Theme.accent, 0.18) : row.hovered ? Theme.surfaceAlt :
                                                                                 "transparent"

        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        Column {
            width: parent.width

            Heading {
                text: qsTr("Notebooks")
            }

            Repeater {
                model: Vault.notebooks

                Entry {
                    id: notebook

                    required property string path
                    required property string name
                    required property int depth
                    required property bool hasChildren
                    required property bool expanded
                    required property int count

                    width: root.width
                    label: name
                    selected: Vault.notes.tag === "" && Vault.notes.notebook === path
                    onClicked: Vault.notes.notebook = path

                    contentItem: Item {
                        AbstractButton {
                            id: fold

                            x: Theme.space4 + notebook.depth * Theme.treeIndent
                            anchors.verticalCenter: parent.verticalCenter
                            width: Theme.smallGlyphSize + Theme.space3
                            height: parent.height
                            visible: notebook.hasChildren
                            Accessible.name: notebook.expanded ? qsTr("Fold %1").arg(notebook.name) :
                                                                 qsTr("Open %1").arg(notebook.name)
                            onClicked: Vault.notebooks.toggle(notebook.path)

                            contentItem: Glyph {
                                name: notebook.expanded ? "chevron-down" : "chevron-right"
                                color: Theme.textFaint
                                stroke: Theme.smallGlyphStroke
                                implicitWidth: Theme.smallGlyphSize
                                implicitHeight: Theme.smallGlyphSize
                            }
                        }
                        Text {
                            anchors {
                                left: fold.right
                                right: countLabel.left
                                rightMargin: Theme.space3
                                verticalCenter: parent.verticalCenter
                            }
                            text: notebook.name
                            elide: Text.ElideRight
                            color: notebook.selected ? Theme.text : Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.textMd
                            font.weight: notebook.selected ? Theme.weightBold : Theme.weightStrong
                        }
                        Text {
                            id: countLabel

                            anchors {
                                right: parent.right
                                rightMargin: Theme.space5
                                verticalCenter: parent.verticalCenter
                            }
                            text: notebook.count
                            color: Theme.textFaint
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.textXs
                            font.features: {
                                "tnum": 1
                            }
                        }
                    }
                }
            }

            Heading {
                visible: Vault.tags.length > 0
                text: qsTr("Tags")
            }

            Repeater {
                model: Vault.tags

                Entry {
                    id: tagRow

                    required property string modelData

                    width: root.width
                    label: "#" + modelData
                    selected: Vault.notes.tag === modelData
                    onClicked: Vault.notes.tag = modelData

                    contentItem: Text {
                        leftPadding: Theme.space5 + tagRow.modelData.split("/").length
                                     * Theme.space1
                        verticalAlignment: Text.AlignVCenter
                        text: tagRow.label
                        elide: Text.ElideRight
                        color: tagRow.selected ? Theme.text : Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.textMd
                        font.weight: tagRow.selected ? Theme.weightBold : Theme.weightStrong
                    }
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
