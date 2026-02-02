import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    id: window
    width: 470
    height: 640
    visible: true
    title: "AutoClicker"

    readonly property bool uiDark: themeSwitch.userModified ? themeSwitch.checked : theme.isDarkMode

    Material.theme: uiDark ? Material.Dark : Material.Light
    Material.accent: Material.Blue

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
