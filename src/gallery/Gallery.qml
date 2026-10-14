pragma ComponentBehavior: Bound

import Jot.Ui
import QtQuick
import QtQuick.Controls

/// Every token and component of the active theme on one page, so a theme can be
/// judged by looking rather than by reading values.
ApplicationWindow {
    id: window

    width: 1100
    height: 900
    visible: true
    title: qsTr("Jot gallery: %1").arg(Theme.name)
    color: Theme.bg

    component Heading: Text {
        color: Theme.text
        font.family: Theme.displayFontFamily
        font.pixelSize: Theme.textXl
        font.weight: Theme.weightStrong
        topPadding: Theme.space6
    }

    component Label: Text {
        color: Theme.textMuted
        font.family: Theme.fontFamily
        font.pixelSize: Theme.textSm
    }

    component Mono: Text {
        color: Theme.textFaint
        font.family: Theme.monoFontFamily
        font.pixelSize: Theme.textXs
        font.features: {
            "tnum": 1
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        Column {
            objectName: "content"
            spacing: Theme.space4
            width: parent.width
            padding: Theme.space7

            Text {
                text: Theme.name
                color: Theme.text
                font.family: Theme.displayFontFamily
                font.pixelSize: Theme.text2xl
                font.weight: Theme.weightStrong
            }

            Label {
                text: Theme.dark ? qsTr("Dark theme") : qsTr("Light theme")
            }

            Repeater {
                model: Theme.warnings

                Text {
                    required property string modelData

                    text: qsTr("Warning: %1").arg(modelData)
                    color: Theme.danger
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.textMd
                }
            }

            Heading {
                text: qsTr("Colors")
            }

            Flow {
                width: parent.width - 2 * Theme.space7
                spacing: Theme.space4

                Repeater {
                    model: Theme.swatches

                    Column {
                        id: swatch

                        required property var modelData

                        spacing: Theme.space2

                        Rectangle {
                            width: 132
                            height: 64
                            radius: Theme.radiusMd
                            color: swatch.modelData.color
                            border.color: Theme.border
                            border.width: 1
                        }

                        Label {
                            text: swatch.modelData.name
                        }

                        Mono {
                            text: swatch.modelData.color.toString() + "  " + Theme.contrast(
                                      swatch.modelData.color, Theme.bg).toFixed(1) + ":1"
                        }
                    }
                }
            }

            Heading {
                text: qsTr("Type")
            }

            Repeater {
                model: [Theme.text2xl, Theme.textXl, Theme.textLg, Theme.textMd, Theme.textSm,
                    Theme.textXs]

                Text {
                    required property int modelData

                    text: qsTr("%1px: Mara keeps the ferry log in a waxed notebook").arg(modelData)
                    color: Theme.text
                    font.family: Theme.fontFamily
                    font.pixelSize: modelData
                }
            }

            Heading {
                text: qsTr("Shape")
            }

            Row {
                spacing: Theme.space5

                Repeater {
                    model: [Theme.radiusSm, Theme.radiusMd, Theme.radiusLg, Theme.radiusXl]

                    Rectangle {
                        id: radiusSample

                        required property int modelData

                        width: 96
                        height: 64
                        radius: modelData
                        color: Theme.surfaceAlt
                        border.color: Theme.border
                        border.width: 1

                        Mono {
                            anchors.centerIn: parent
                            text: radiusSample.modelData + "px"
                        }
                    }
                }
            }

            Heading {
                text: qsTr("Controls")
            }

            Row {
                spacing: Theme.space3

                PillButton {
                    label: qsTr("Notes")
                    selected: true
                }

                PillButton {
                    label: qsTr("Outline")
                }

                StickerButton {
                    glyph: "chevron-left"
                    Accessible.name: qsTr("Previous")
                }

                StickerButton {
                    glyph: "chevron-right"
                    Accessible.name: qsTr("Next")
                }

                StickerButton {
                    text: qsTr("Search")
                }

                StickerButton {
                    text: qsTr("New note")
                    accent: true
                }

                // Disabled lies flat and grey, accent or not.
                StickerButton {
                    text: qsTr("Disabled")
                    glyph: "plus"
                    accent: true
                    enabled: false
                }

                Dropdown {
                    model: [
                        {
                            "id": "updated",
                            "name": qsTr("Updated")
                        },
                        {
                            "id": "title",
                            "name": qsTr("Title")
                        }
                    ]
                    currentValue: "updated"
                    minimumWidth: 140
                }

                WindowControls {
                    buttons: ["minimize", "maximize", "close"]
                }
            }
        }
    }
}
