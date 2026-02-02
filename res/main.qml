import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    id: window
    width: 430
    height: 640
    visible: true
    title: "AutoClicker"
    font.pointSize: 16

    property bool uiDark: themeSwitch.userModified ? themeSwitch.checked : theme.isDarkMode
    property color textColor: Material.foreground
    property color borderColor: Material.color(Material.Grey, Material.Shade400)

    Material.theme: uiDark ? Material.Dark : Material.Light
    Material.accent: Material.Blue

    Rectangle {
        anchors.top: parent.top
        anchors.topMargin: 70
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        height: 70
        border.width: 1
        border.color: borderColor
        radius: 4
        color: Material.background

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            Label {
                text: "HotKey"
                color: textColor
            }
            Item {
                Layout.fillWidth: true
            }
            Button {
                text: "F1"
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
