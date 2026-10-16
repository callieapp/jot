import Jot.Ui
import QtQuick

/// The border between two columns, which drags to resize the one on its left. A double
/// click puts it back to the theme's width.
Item {
    id: root

    /// The width the column has now.
    required property real columnWidth

    /// The column should be this wide.
    signal resized(int width)
    /// The column should go back to the theme's width.
    signal reset

    width: Theme.space3
    // Over both columns' edges, so it is easy to catch.
    z: 10

    HoverHandler {
        cursorShape: Qt.SizeHorCursor
    }

    DragHandler {
        id: drag

        property real startWidth

        target: null
        // The rows under it drag too; once resizing starts, none of them may take over.
        grabPermissions: PointerHandler.CanTakeOverFromAnything
        xAxis.enabled: true
        yAxis.enabled: false
        onActiveChanged: {
            if (active)
                startWidth = root.columnWidth
        }
        onActiveTranslationChanged: {
            if (active)
                root.resized(Math.round(startWidth + activeTranslation.x))
        }
    }

    TapHandler {
        onDoubleTapped: root.reset()
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        width: 2
        height: parent.height
        visible: drag.active
        color: Theme.accent
    }
}
