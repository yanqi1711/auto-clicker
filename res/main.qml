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

    property bool uiDark: themeSwitch.userModified ? themeSwitch.checked : theme.isDarkMode
    property color textColor: Material.foreground
    property color borderColor: Material.color(Material.Grey, Material.Shade400)

    Material.theme: uiDark ? Material.Dark : Material.Light
    Material.accent: Material.Blue
    Material.background: uiDark ? "#191919" : "#ffffff"

    ButtonGroup {
        id: mainButtonGroup
        exclusive: true
    }

    Component {
        id: settingRowComponent
        Rectangle {
            id: rowRoot
            property string labelText: ""
            property string buttonText: ""
            property var btnGroup: null

            width: parent.width
            height: 60
            border.width: 1
            border.color: borderColor
            radius: 3
            color: Material.background

            signal clicked

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10

                Label {
                    text: labelText
                    color: textColor
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: customBtn
                    text: buttonText // 如果正在录制且选中，显示省略号
                    onClicked: rowRoot.clicked()
                    checkable: true
                    ButtonGroup.group: btnGroup

                    contentItem: Text {
                        text: customBtn.text
                        font.pointSize: 14
                        font.bold: customBtn.checked
                        color: customBtn.checked ? "#fff" : textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 80
                        implicitHeight: 36
                        color: customBtn.checked ? "#6872ab" : Material.background
                        border.color: customBtn.checked ? "#6872ab" : borderColor
                        border.width: 1
                        radius: 3
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
            id: hotkeyLoader
            width: parent.width
            sourceComponent: settingRowComponent
            onLoaded: {
                item.labelText = "HotKey";
                item.buttonText = Qt.binding(() => cfg.hotkey); // 动态绑定 C++ 属性
                item.btnGroup = mainButtonGroup;
                item.clicked.connect(function () {
                    cfg.startRecording("hotkey");
                // 这里的录制逻辑交由 C++ 的 setRecording(true) 去拦截键盘
                // QML 这边只需要把按钮设为 checked 即可
                });
            }
        }

        Loader {
            id: simulateKeyLoader
            width: parent.width
            sourceComponent: settingRowComponent
            onLoaded: {
                item.labelText = "Simulate key";
                item.buttonText = Qt.binding(() => cfg.simulateKey);
                item.btnGroup = mainButtonGroup;
                item.clicked.connect(function () {
                    cfg.startRecording("simulateKey");
                });
            }
        }

        Loader {
            id: intervalLoader
            width: parent.width
            sourceComponent: settingRowComponent
            onLoaded: {
                item.labelText = "Click Interval(ms)";
                item.buttonText = Qt.binding(() => cfg.interval.toString());
                item.btnGroup = mainButtonGroup;
                item.clicked.connect(function () {
                    bottomPanel.open();
                });
            }
        }
    }

    // 弹窗
    Popup {
        id: bottomPanel
        x: 0
        width: parent.width
        height: parent.height / 2
        y: parent.height
        padding: 0
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            TextField {
                id: intervalInput
                placeholderText: "输入间隔 (ms)"
                text: cfg.interval.toString()
                Layout.fillWidth: true
            }
            Button {
                text: "确定"
                Layout.fillWidth: true
                onClicked: {
                    cfg.interval = parseInt(intervalInput.text);
                    bottomPanel.close();
                }
            }
        }

        Overlay.modal: Rectangle {
            color: "#88000000"
        }

        enter: Transition {
            NumberAnimation {
                property: "y"
                from: window.height
                to: window.height * 0.5
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
        exit: Transition {
            NumberAnimation {
                property: "y"
                from: window.height * 0.5
                to: window.height
                duration: 200
                easing.type: Easing.InQuad
            }
        }

        background: Rectangle {
            color: Material.background
            radius: 12
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 12
                color: parent.color
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
