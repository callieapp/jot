pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Under a note: the lines in other notes that link to it, and the notes that name it
/// without a link. Each opens its note.
Column {
    id: root

    spacing: Theme.space2

    component Heading: Text {
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
        id: entry

        required property string path
        required property string title
        required property string excerpt
        required property bool mention

        width: root.width
        height: content.implicitHeight + 2 * Theme.space3
        hoverEnabled: true
        Accessible.name: title
        onClicked: Vault.current = path

        background: Rectangle {
            radius: Theme.radiusMd
            color: entry.hovered ? Theme.surfaceAlt : Theme.surface
            border.color: Theme.hairline
        }

        contentItem: Column {
            id: content

            leftPadding: Theme.space4
            rightPadding: Theme.space4
            topPadding: Theme.space3
            spacing: Theme.space1

            Text {
                width: entry.width - 2 * Theme.space4
                text: entry.title
                elide: Text.ElideRight
                color: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textMd
                font.weight: Theme.weightBold
            }
            Text {
                width: entry.width - 2 * Theme.space4
                visible: entry.excerpt !== ""
                text: entry.excerpt
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textSm
            }
        }
    }

    Rectangle {
        width: root.width
        height: 1
        color: Theme.hairline
    }

    Heading {
        text: Vault.backlinks.linked === 0 ? qsTr("No links to this note") : Vault.backlinks.linked
                                             === 1 ? qsTr("1 link to this note") : qsTr(
                                                         "%1 links to this note").arg(
                                                         Vault.backlinks.linked)
    }

    Repeater {
        model: Vault.backlinks

        Entry {
            visible: !mention
        }
    }

    Heading {
        visible: Vault.backlinks.unlinked > 0
        text: qsTr("Mentioned without a link")
    }

    Repeater {
        model: Vault.backlinks

        Entry {
            visible: mention
        }
    }
}
