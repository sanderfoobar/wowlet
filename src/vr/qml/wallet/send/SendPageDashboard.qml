import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "../../common"

ColumnLayout {
	id: root

    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        text: "How to transfer Wownero?"
    }

    RowLayout {
        Layout.topMargin: 30
        Layout.fillWidth: true
        spacing: 30

    	Rectangle {
    		color: sendStateController.exitedColor
    		Layout.fillWidth: true
    		Layout.fillHeight: true

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.color = sendStateController.enteredColor
                onExited: parent.color = sendStateController.exitedColor
                onPressed: parent.color = sendStateController.pressedColor
                onClicked: {
                    sendStateView.state = "pinPage";
                }
            }
    	}

    	Rectangle {
    		color: sendStateController.exitedColor
    		Layout.fillWidth: true
    		Layout.fillHeight: true

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.color = sendStateController.enteredColor
                onExited: parent.color = sendStateController.exitedColor
                onPressed: parent.color = sendStateController.pressedColor
                onClicked: {
                    sendStateView.state = "qrPage";
                }
            }
    	}
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    function onPageCompleted(previousView){

    }
}