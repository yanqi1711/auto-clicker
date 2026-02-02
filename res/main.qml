import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    id: window
    width: 420
    height: 540
    visible: true
    title: "AutoClicker"
    font.pointSize: 16
    font.bold: true

    property bool uiDark: themeSwitch.userModified ? themeSwitch.checked : theme.isDarkMode
    property color textColor: Material.foreground
    property color borderColor: Material.color(Material.Grey, Material.Shade400)

    Material.theme: uiDark ? Material.Dark : Material.Light
    Material.accent: Material.Blue

    Component {
        id: settingRowComponent
        Rectangle {
            property string labelText: ""
            property string buttonText: ""

            width: parent.width
            height: 60
            border.width: 1
            border.color: borderColor
            radius: 4
            color: Material.background

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10

                Label {
                    text: labelText
                    color: textColor
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: customBtn
                    text: buttonText

                    leftPadding: 5
                    rightPadding: 5
                    topPadding: 5
                    bottomPadding: 5

                    contentItem: Text {
                        text: customBtn.text
                        font: window.font
                        color: textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        anchors.fill: parent

                        color: customBtn.pressed ? Material.color(Material.Grey, Material.Shade300) : "transparent"
                        border.color: '#3fa8cb'
                        border.width: 1
                        radius: 4
                    }
                }
            }
        }
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: 70
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 14

        Loader {
            width: parent.width
            sourceComponent: settingRowComponent
            onLoaded: {
                item.labelText = "HotKey";
                item.buttonText = "F1";
            }
        }

        Loader {
            width: parent.width
            sourceComponent: settingRowComponent
            onLoaded: {
                item.labelText = "Simulate key";
                item.buttonText = "LeftMouseBtn";
            }
        }

        Loader {
            width: parent.width
            sourceComponent: settingRowComponent
            onLoaded: {
                item.labelText = "Click Interval(ms)";
                item.buttonText = "100";
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 10

        Button {
            id: themeSwitch
            Material.accent: Material.darkgray
            width: 24
            height: 24
            background: Rectangle {
                color: "transparent"
            }

            property bool userModified: false
            checked: theme.isDarkMode

            onClicked: {
                userModified = true;
                checked = !checked;
            }

            Image {
                id: themeIcon
                anchors.fill: parent
                source: uiDark ? "qrc:/public/IcTwotoneDarkMode.svg" : "qrc:/public/IcTwotoneLightMode.svg"
                fillMode: Image.PreserveAspectFit
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}
