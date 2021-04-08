import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import "." // QTBUG-34418, singletons require explicit import to load qmldir file

Item {
    id: root
    property string text: "-"
    property string iconPath: ""
    property bool hasIcon: iconPath !== ""
    property bool hoverEnabled: true
    property bool activationSoundEnabled: true
    property string enteredColor: Style.btnEnteredColor
    property string exitedColor: Style.btnExitedColor
    property string pressedColor: Style.btnPressedColor
    signal clicked;

    Layout.preferredHeight: 70

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        Layout.minimumHeight: 54

        // Image {
        //     visible: hasIcon
        //     source: iconPath
        //     Layout.leftMargin: 20
        //     Layout.rightMargin: 20
        //     Layout.preferredWidth: 32
        //     Layout.preferredHeight: 32
        // }

        ImageMask {
            id: backIcon
            visible: hasIcon
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32

            image: iconPath
            color: Style.fontColorBright
        }

        MyText {
            id: btnText
            Layout.leftMargin: root.hasIcon ? 0 : 20
            Layout.rightMargin: root.hasIcon ? 0 : 20
            Layout.fillWidth: root.hasIcon ? true : false
            Layout.alignment: root.hasIcon ? Qt.AlignLeft : Qt.AlignHCenter
            text: root.text
            fontSize: 16
            fontColor: Style.btnTextColor
        }
    }

    Rectangle {
        z: root.z - 1
        anchors.fill: root
        Layout.fillWidth: root.Layout.fillWidth
        color: root.down ? Style.btnPressedColor : (root.activeFocus ? Style.btnEnteredColor : Style.btnExitedColor)

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                parent.color = Style.btnEnteredColor;
                btnText.fontColor = Style.btnTextHoverColor;
            }
            onExited: {
                parent.color = Style.btnExitedColor;
                btnText.fontColor = Style.btnTextColor;
            }
            onPressed: {
                parent.color = Style.btnPressedColor;
                btnText.fontColor = Style.btnTextHoverColor;
            }
            onClicked: {
                root.clicked();
            }
        }
    }
}