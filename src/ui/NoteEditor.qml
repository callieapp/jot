pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// The note shown, to read and to write. Its title renames the file; its body is the
/// Markdown itself, styled as it is typed. Ctrl+click follows a link, and a click on a
/// task's box ticks it.
Rectangle {
    id: root

    /// Puts the cursor in the title, all selected, as for a note just made.
    function editTitle() {
        title.forceActiveFocus()
        title.selectAll()
    }

    /// Puts the cursor at the end of the body.
    function editBody() {
        body.forceActiveFocus()
        body.cursorPosition = body.length
    }

    color: Theme.bg

    // The editor holds the text; the vault hears of each change and saves it shortly after.
    // A note loaded from disk replaces the text without counting as an edit.
    property bool loading: false

    function load() {
        loading = true
        const cursor = body.cursorPosition
        // Swapping a whole note in under a live highlighter leaves the TextEdit blank, so
        // the styling comes off for the swap and goes back on after.
        styler.document = null
        body.text = Vault.currentText
        styler.document = body.textDocument
        body.cursorPosition = Math.min(cursor, body.length)
        title.text = Vault.currentTitle
        loading = false
    }

    Connections {
        target: Vault
        function onCurrentChanged() {
            if (Vault.currentText !== body.text || Vault.currentTitle !== title.text)
                root.load()
        }
    }
    Component.onCompleted: load()

    Text {
        anchors.centerIn: parent
        visible: Vault.current === ""
        horizontalAlignment: Text.AlignHCenter
        text: Vault.noteCount === 0 ? qsTr(
                                          "This vault has no notes yet.\nPress Ctrl+N to make one.") :
                                      qsTr("Choose a note, or press Ctrl+P to find one.")
        color: Theme.textFaint
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textBase
    }

    Flickable {
        id: flick

        anchors.fill: parent
        visible: Vault.current !== ""
        contentWidth: width
        contentHeight: page.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        /// Scrolls so the cursor stays in view while typing.
        function follow(rect) {
            const top = page.y + body.y + rect.y
            const bottom = top + rect.height
            if (top < contentY)
                contentY = Math.max(0, top - Theme.space6)
            else if (bottom > contentY + height)
                contentY = bottom - height + Theme.space6
        }

        Column {
            id: page

            width: Math.min(flick.width, Theme.editorWidth + 2 * Theme.space7)
            x: (flick.width - width) / 2
            padding: Theme.space7
            spacing: Theme.space5

            Row {
                width: parent.width - 2 * parent.padding
                spacing: Theme.space3

                TextInput {
                    id: title

                    width: parent.width - trash.width - parent.spacing
                    readOnly: !Vault.writable
                    wrapMode: TextInput.Wrap
                    color: Theme.text
                    selectionColor: Theme.tint(Theme.accent, 0.4)
                    selectedTextColor: Theme.text
                    font.family: Theme.displayFontFamily
                    font.pixelSize: Theme.text2xl
                    font.weight: Theme.weightBold
                    selectByMouse: true
                    Accessible.name: qsTr("Title")
                    onEditingFinished: {
                        if (text.trim() === "")
                            text = Vault.currentTitle
                        else if (text !== Vault.currentTitle)
                            Vault.renameCurrent(text)
                    }
                    Keys.onReturnPressed: root.editBody()
                    Keys.onEscapePressed: {
                        text = Vault.currentTitle
                        root.editBody()
                    }
                }
                DangerButton {
                    id: trash

                    anchors.verticalCenter: title.verticalCenter
                    visible: Vault.writable
                    label: qsTr("Delete")
                    armedLabel: qsTr("Move to trash?")
                    onConfirmed: Vault.trashCurrent()
                }
            }

            Text {
                width: parent.width - 2 * parent.padding
                visible: Vault.problem !== ""
                text: Vault.problem
                wrapMode: Text.WordWrap
                color: Theme.danger
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textMd
                font.weight: Theme.weightStrong
            }

            TextEdit {
                id: body

                objectName: "body"

                width: parent.width - 2 * parent.padding
                readOnly: !Vault.writable
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                color: Theme.text
                selectionColor: Theme.tint(Theme.accent, 0.4)
                selectedTextColor: Theme.text
                font.family: Theme.fontFamily
                font.pixelSize: Theme.textLg
                selectByMouse: true
                persistentSelection: true
                tabStopDistance: 4 * Theme.space5
                Accessible.name: qsTr("Note")
                onTextChanged: {
                    if (!root.loading)
                        Vault.edit(text)
                }
                onCursorRectangleChanged: flick.follow(cursorRectangle)
                onContentHeightChanged: layoutTick++
                onWidthChanged: layoutTick++

                /// Bumped when lines may have moved, so the images find their lines again.
                property int layoutTick: 0

                /// Puts `markdown` on lines of its own at `position`.
                function insertBlock(position, markdown) {
                    const place = Vault.placeBlock(text, position, markdown)
                    insert(place.position, place.text)
                    cursorPosition = place.cursor
                }

                Keys.onPressed: event => {
                    if (!event.matches(StandardKey.Paste))
                        return
                    // An image on the clipboard becomes an attachment; anything else pastes as text.
                    const markdown = Vault.pasteImage()
                    if (markdown === "")
                        return
                    insertBlock(cursorPosition, markdown)
                    event.accepted = true
                }

                MarkdownStyler {
                    id: styler

                    cursorPosition: body.activeFocus ? body.cursorPosition : -1
                    pixelSize: body.font.pixelSize
                    width: body.width
                }

                Repeater {
                    model: styler.images

                    Image {
                        required property var modelData

                        readonly property rect line: {
                            body.layoutTick
                            return body.positionToRectangle(modelData.position)
                        }

                        x: 0
                        y: line.y
                        width: modelData.width
                        height: modelData.height
                        source: modelData.source
                        sourceSize: Qt.size(modelData.width * 2, modelData.height * 2)
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        smooth: true
                        mipmap: true
                    }
                }

                DropArea {
                    anchors.fill: parent
                    enabled: !body.readOnly
                    keys: ["text/uri-list"]
                    onDropped: drop => {
                        const markdown = Vault.attachFiles(drop.urls)
                        if (markdown === "")
                            return
                        body.insertBlock(body.positionAt(drop.x, drop.y), markdown)
                        drop.accept(Qt.CopyAction)
                    }
                }

                TapHandler {
                    acceptedModifiers: Qt.ControlModifier
                    onTapped: point => {
                        const hit = styler.hitAt(body.positionAt(point.position.x,
                                                                 point.position.y))

                        if (hit.kind === "wikilink" || hit.kind === "link" || hit.kind === "url")
                            Vault.follow(hit.target, hit.kind === "wikilink")
                    }
                }
                TapHandler {
                    acceptedModifiers: Qt.NoModifier
                    enabled: !body.readOnly
                    onTapped: point => {
                        const hit = styler.hitAt(body.positionAt(point.position.x,
                                                                 point.position.y))

                        if (hit.kind !== "task")
                            return
                        const done = body.getText(hit.position, hit.position + 1) !== " "
                        body.remove(hit.position, hit.position + 1)
                        body.insert(hit.position, done ? " " : "x")
                    }
                }
                HoverHandler {
                    id: hover

                    cursorShape: Qt.IBeamCursor
                    onPointChanged: previewTimer.restart()
                    onHoveredChanged: {
                        if (!hovered) {
                            previewTimer.stop()
                            preview.close()
                        }
                    }
                }

                // A link the pointer rests on shows the start of its note.
                Timer {
                    id: previewTimer

                    interval: 450
                    onTriggered: {
                        const at = body.positionAt(hover.point.position.x, hover.point.position.y)
                        const hit = styler.hitAt(at)
                        if (hit.kind !== "wikilink" && hit.kind !== "link") {
                            preview.close()
                            return
                        }
                        const note = Vault.preview(hit.target)
                        if (!note.path) {
                            preview.close()
                            return
                        }
                        preview.title = note.title
                        preview.text = note.text
                        preview.x = Math.min(hover.point.position.x, body.width - preview.width)
                        preview.y = body.positionToRectangle(at).y + body.positionToRectangle(
                                    at).height + Theme.space2
                        preview.open()
                    }
                }

                LinkPreview {
                    id: preview
                }
            }

            Backlinks {
                width: parent.width - 2 * parent.padding
            }
        }
    }
}
