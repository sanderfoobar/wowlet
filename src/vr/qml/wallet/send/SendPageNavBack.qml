import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

import "../../common"


RowLayout {
    id: root
    Layout.fillWidth: true
    Layout.preferredHeight: 82
    Layout.maximumHeight: 82

    signal backClicked();

    Rectangle {
        color: sendStateController.exitedColor
        Layout.preferredHeight: 82
        Layout.preferredWidth: 220

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            Layout.minimumHeight: 54

            Image {
                source: "qrc:/backarrow"
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
            }

            MyText {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                fontBold: true
                text: "back"
                fontSize: 16
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: parent.color = sendStateController.enteredColor
            onExited: parent.color = sendStateController.exitedColor
            onPressed: parent.color = sendStateController.pressedColor
            onClicked: {
                root.backClicked();
            }
        }
    }
}