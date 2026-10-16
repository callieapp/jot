import Jot.Ui
import QtQuick

/// Under the title of a note that is a task: whether it is done, and when it is due.
Row {
    id: root

    /// The note's front matter, from the styler.
    property var frontMatter: ({})

    /// Asks for `action` (done or due), with `argument` the due date or empty.
    signal formatRequested(string action, string argument)

    readonly property bool done: frontMatter.task === "done"
    readonly property date due: {
        const match = /^(\d{4})-(\d{2})-(\d{2})/.exec(frontMatter.due || "")
        return match ? new Date(match[1], match[2] - 1, match[3]) : new Date(NaN)
    }

    spacing: Theme.space2

    Toggle {
        anchors.verticalCenter: parent.verticalCenter
        text: qsTr("Done")
        // Follows the note: a click asks for the change, and the text says what it is.
        checked: root.done
        focusPolicy: Qt.NoFocus
        onToggled: {
            checked = Qt.binding(() => root.done)
            root.formatRequested("done", "")
        }
    }
    DatePicker {
        anchors.verticalCenter: parent.verticalCenter
        day: root.due
        placeholder: qsTr("Add a due date")
        focusPolicy: Qt.NoFocus
        onPicked: day => root.formatRequested("due", Qt.formatDate(day, "yyyy-MM-dd"))
    }
    StickerButton {
        anchors.verticalCenter: parent.verticalCenter
        visible: !isNaN(root.due.getTime())
        glyph: "close"
        focusPolicy: Qt.NoFocus
        Accessible.name: qsTr("No due date")
        tip: Accessible.name
        onClicked: root.formatRequested("due", "")
    }
}
