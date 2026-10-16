import Jot.Ui
import QtQuick
import QtQuick.Controls

/// A menu in the theme's card style, for MenuEntry items.
Menu {
    id: root

    /// The narrowest the menu gets, whatever its entries.
    property real minimumWidth: 200

    padding: Theme.space2
    // A submenu's own entry, in the parent menu, looks like the rest.
    delegate: MenuEntry {}

    background: Rectangle {
        implicitWidth: root.minimumWidth
        color: Theme.surface
        border.color: Theme.border
        radius: Theme.radiusLg

        SoftShadow {
            anchors.fill: parent
            radius: parent.radius
        }
    }
}
