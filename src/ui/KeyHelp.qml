pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Every keyboard shortcut, from the table the window's shortcuts are made from: those
/// that always work, then vi mode's keys and the commands after its leader.
Popup {
    id: root

    /// KeyRouter's actions: {label, scope, keys, vi, leader} for each.
    required property var actions

    /// The actions of `scope` that have keys of `kind` ("keys" or "vi"), as rows.
    function rows(scope, kind) {
        return actions.filter(a => a.scope === scope && a[kind].length > 0).map(a => ({
            "keys": kind === "vi" ? [a.vi] : a.keys,
            "label": a.label
        }))
    }

        anchors.centerIn: Overlay.overlay
        width: 620
        // The overlay, not the parent: in a LazyPopup the parent is a loader with no size.
        height: Math.min(implicitHeight, (Overlay.overlay ? Overlay.overlay.height : 800) - 2
        * Theme.space6)
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

        contentItem: Flickable {
        implicitHeight: content.implicitHeight
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
        id: content
        width: parent.width
        spacing: Theme.space4

        Text {
        text: qsTr("Keyboard shortcuts")
        color: Theme.text
        font.family: Theme.displayFontFamily
        font.pixelSize: Theme.textXl
        font.weight: Theme.weightBold
    }

        Table {
        title: qsTr("Anywhere")
        rows: root.rows("window", "keys").concat(root.rows("list", "keys"))
    }
        Table {
        title: qsTr("While writing")
        rows: root.rows("note", "keys")
    }

        Text {
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
        title: qsTr("Vi mode, out of the note")
        rows: root.rows("window", "vi")
    }
        Table {
        title: qsTr("Vi mode, normal mode in the note")
        rows: root.rows("note", "vi")
    }
        Table {
        title: qsTr("Vi mode, visual mode")
        rows: root.rows("visual", "vi").concat([
        {
        "keys": ["h", "j", "k", "l"],
        "label": qsTr("Select, by normal mode's moves")
    }
        ])
    }
        Table {
        title: qsTr("After the leader (%1)").arg(Settings.leaderKey === " " ? qsTr("Space") :
        Settings.leaderKey)
        rows: root.actions.filter(a => a.leader !== "").map(a => ({
        "keys": [a.leader],
        "label": a.label
    }))
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
