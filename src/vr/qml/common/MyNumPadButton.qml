import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Rectangle {
    id: root
    property string text: ""
    property int fontSize: 22
    property string enteredColor: "#365473"
    property string exitedColor: "#2c435d"
    property string pressedColor: "#406288"
    property alias mouseArea: mouseArea

    signal clicked;

    Layout.preferredWidth: 92
    Layout.preferredHeight: 92
    color: root.exitedColor

    MyText {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        color: "white"
        text: root.text
        fontSize: root.fontSize
        fontBold: true
    }

    MouseArea {
        id: mouseArea
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