import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import "." // QTBUG-34418, singletons require explicit import to load qmldir file

Item {
    id: root
    property string text: "testy"
    property string iconPath: ""
    property bool hasIcon: iconPath !== ""
    property bool hoverEnabled: true
    property bool activationSoundEnabled: true
    property string enteredColor: "#365473"
    property string exitedColor: "#2c435d"
    property string pressedColor: "#406288"
    signal clicked;

    Layout.preferredHeight: 70

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        Layout.minimumHeight: 54

        Image {
            visible: hasIcon
            source: iconPath
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
        }

        MyText {
            Layout.leftMargin: root.hasIcon ? 0 : 20
            Layout.rightMargin: root.hasIcon ? 0 : 20
            Layout.fillWidth: root.hasIcon ? true : false
            Layout.alignment: root.hasIcon ? Qt.AlignLeft : Qt.AlignHCenter
            text: root.text
            fontSize: 16
        }
    }

    Rectangle {
        z: root.z - 1
        anchors.fill: root
        Layout.fillWidth: root.Layout.fillWidth
        color: root.down ? root.pressedColor : (root.activeFocus ? root.enteredColor : root.exitedColor)

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.color = root.enteredColor
            onExited: parent.color = root.exitedColor
            onPressed: parent.color = root.pressedColor
            onClicked: {
                root.clicked();
            }
        }
    }
}