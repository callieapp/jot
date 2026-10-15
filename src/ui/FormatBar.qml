pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The row over a note: Markdown formatting for the selection or the line the cursor is
/// on, and on the right, whether the note is a to-do.
Rectangle {
    id: root

    /// The note's front matter, from the styler.
    property var frontMatter: ({})

    signal formatRequested(string action)

    readonly property string todo: frontMatter.todo || ""

    implicitHeight: row.implicitHeight + 2 * Theme.space2
    color: Theme.bg

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: Theme.hairline
    }

    Flow {
        id: row

        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: Theme.space5
            rightMargin: todoButton.width + 2 * Theme.space5
        }
        spacing: Theme.space1

        FormatButton {
            label: "H1"
            format: "h1"
            tip: qsTr("Heading (Ctrl+1)")
        }
        FormatButton {
            label: "H2"
            format: "h2"
            tip: qsTr("Smaller heading (Ctrl+2)")
        }
        FormatButton {
            label: "H3"
            format: "h3"
            tip: qsTr("Smallest heading (Ctrl+3)")
        }
        Divider {}
        FormatButton {
            label: "B"
            bold: true
            format: "bold"
            tip: qsTr("Bold (Ctrl+B)")
        }
        FormatButton {
            label: "I"
            italic: true
            format: "italic"
            tip: qsTr("Italic (Ctrl+I)")
        }
        FormatButton {
            label: "S"
            strike: true
            format: "strike"
            tip: qsTr("Strikethrough (Ctrl+Shift+X)")
        }
        FormatButton {
            label: "<>"
            mono: true
            format: "code"
            tip: qsTr("Code (Ctrl+E)")
        }
        FormatButton {
            label: qsTr("Link")
            underline: true
            format: "link"
            tip: qsTr("Link (Ctrl+K)")
        }
        Divider {}
        FormatButton {
            glyph: "list-bullet"
            format: "bullet"
            tip: qsTr("Bulleted list")
        }
        FormatButton {
            glyph: "list-number"
            format: "numbered"
            tip: qsTr("Numbered list")
        }
        FormatButton {
            glyph: "task"
            format: "task"
            tip: qsTr("Checkbox (Ctrl+Enter)")
        }
        FormatButton {
            glyph: "quote"
            format: "quote"
            tip: qsTr("Quote")
        }
        FormatButton {
            glyph: "table"
            format: "table"
            tip: qsTr("Table, or line up the one here")
        }
        FormatButton {
            label: "{ }"
            mono: true
            format: "codeblock"
            tip: qsTr("Code block")
        }
    }

    // A note that is a to-do shows in To-dos until it is done.
    PillButton {
        id: todoButton

        anchors {
            right: parent.right
            rightMargin: Theme.space5
            top: row.top
        }
        label: qsTr("To-do")
        tip: root.todo === "" ? qsTr("Make this note a to-do") : qsTr("No longer a to-do")
        selected: root.todo !== ""
        focusPolicy: Qt.NoFocus
        onClicked: root.formatRequested("todo")
    }

    /// A small button that formats: a label drawn in the style it gives, or a glyph.
    component FormatButton: AbstractButton {
        id: button

        property string label
        property string glyph
        property string format
        property string tip
        property bool bold
        property bool italic
        property bool strike
        property bool underline
        property bool mono

        implicitWidth: Math.max(implicitHeight, caption.implicitWidth + 2 * Theme.space2)
        // As tall as the To-do pill, so the row lines up.
        implicitHeight: 32
        focusPolicy: Qt.NoFocus
        Accessible.name: tip
        onClicked: root.formatRequested(format)

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        background: Rectangle {
            radius: Theme.radiusSm
            color: button.pressed ? Theme.border : button.hovered ? Theme.surfaceAlt : Theme.tint(
                                                                        Theme.surfaceAlt, 0)
        }
        contentItem: Item {
            Text {
                id: caption
                anchors.centerIn: parent
                visible: button.glyph === ""
                text: button.label
                color: button.hovered ? Theme.text : Theme.textMuted
                font.family: button.mono ? Theme.monoFontFamily : Theme.fontFamily
                font.pixelSize: Theme.textMd
                font.weight: button.bold ? Theme.weightHeavy : Theme.weightBold
                font.italic: button.italic
                font.strikeout: button.strike
                font.underline: button.underline
            }
            Glyph {
                anchors.centerIn: parent
                visible: button.glyph !== ""
                name: button.glyph
                color: caption.color
                stroke: Theme.fineGlyphStroke
            }
        }

        Loader {
            active: button.tip !== "" && button.hovered

            sourceComponent: Tip {
                parent: button
                visible: !button.pressed
                text: button.tip
                delay: Theme.durSlow * 2
                closePolicy: Popup.NoAutoClose
            }
        }
    }

    component Divider: Item {
        width: Theme.space3
        height: 32

        Rectangle {
            anchors.centerIn: parent
            width: 1
            height: 18
            color: Theme.border
        }
    }
}
