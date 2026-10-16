pragma ComponentBehavior: Bound
import Jot.Ui
import QtQuick
import QtQuick.Controls

/// A choice between two modes, as a slider: the accent slides under the one chosen.
Rectangle {
    id: root

    /// The two choices: {label, tip} for each.
    required property var options
    /// Which is chosen, 0 or 1.
    property int current: 0

    signal picked(int index)

    readonly property real segment: Math.max(first.implicitWidth, second.implicitWidth)

    implicitWidth: 2 * segment + 2 * inset
    implicitHeight: 26
    radius: height / 2
    color: Theme.surfaceAlt

    readonly property real inset: 2

    Rectangle {
        id: thumb

        objectName: "thumb"
        x: root.inset + root.current * root.segment
        y: root.inset
        width: root.segment
        height: root.height - 2 * root.inset
        radius: height / 2
        color: Theme.accent

        Behavior on x {
            NumberAnimation {
                duration: Theme.durMed
                easing.type: Theme.easingBounce
                easing.overshoot: Theme.bounce
            }
        }
    }

    Row {
        x: root.inset
        y: root.inset
        height: root.height - 2 * root.inset

        Choice {
            id: first
            index: 0
        }
        Choice {
            id: second
            index: 1
        }
    }

    component Choice: AbstractButton {
        id: choice

        required property int index
        readonly property bool chosen: root.current === index
        readonly property var option: root.options[index]

        width: root.segment
        height: parent.height
        implicitWidth: caption.implicitWidth + 2 * Theme.space3
        text: option.label
        focusPolicy: Qt.NoFocus
        Accessible.role: Accessible.RadioButton
        Accessible.checked: chosen
        onClicked: root.picked(index)

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        contentItem: Text {
            id: caption
            text: choice.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: choice.chosen ? Theme.accentText : choice.hovered ? Theme.text : Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textSm
            font.weight: choice.chosen ? Theme.weightHeavy : Theme.weightBold

            Behavior on color {
                ColorAnimation {
                    duration: Theme.durFast
                }
            }
        }
        background: null

        // What the choice does, and its shortcut, after resting on it a moment.
        Loader {
            active: choice.option.tip !== "" && choice.hovered

            sourceComponent: Tip {
                parent: choice
                visible: !choice.pressed
                text: choice.option.tip
                delay: Theme.durSlow * 2
                closePolicy: Popup.NoAutoClose
            }
        }
    }
}
