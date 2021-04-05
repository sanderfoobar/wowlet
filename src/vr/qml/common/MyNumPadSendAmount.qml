import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "."

RowLayout {
    id: root
    property double amount: 0.0;
    property bool add: true;

    property string enteredColor: "#365473"
    property string exitedColor: "#2c435d"
    property string pressedColor: "#406288"

    signal amountUpdated(double amount);

    spacing: 24

    function reset() {
        root.amount = 0.0;
    }

    MyNumPadButton {
        id: minButton
        mouseArea.enabled: false
        fontSize: 18

        Layout.preferredWidth: 128
        Layout.preferredHeight: 112

        color: root.add ? root.exitedColor : root.enteredColor

        text: "-"

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                if(!root.add) return;
                parent.color = root.enteredColor
            }
            onExited: {
                if(!root.add) return;
                parent.color = root.exitedColor
            }
            onClicked: {
                root.add = false;
                plusButton.color = root.exitedColor
            }
        }
    }

    MyNumPadButton {
        id: plusButton
        mouseArea.enabled: false
        fontSize: 18

        Layout.preferredWidth: 128
        Layout.preferredHeight: 112

        color: root.add ? root.enteredColor : root.exitedColor

        text: "+"

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                if(root.add) return;
                parent.color = root.enteredColor
            }
            onExited: {
                if(root.add) return;
                parent.color = root.exitedColor
            }
            onClicked: {
                root.add = true;
                minButton.color = root.exitedColor
            }
        }
    }

    Rectangle {
        color: "#cccccc"
        width: 1
        Layout.fillHeight: true
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 180
        Layout.preferredHeight: 112
        text: "0.001"
        onClicked: {
            root.add ? root.amount += 0.001 : root.amount -= 0.001;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 148
        Layout.preferredHeight: 112
        text: "0.01"
        onClicked: {
            root.add ? root.amount += 0.01 : root.amount -= 0.01;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 128
        Layout.preferredHeight: 112
        text: "0.1"
        onClicked: {
            root.add ? root.amount += 0.1 : root.amount -= 0.1;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 128
        Layout.preferredHeight: 112
        text: "1"
        onClicked: {
            root.add ? root.amount += 1.0 : root.amount -= 1.0;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 168
        Layout.preferredHeight: 112
        text: "10"
        onClicked: {
            root.add ? root.amount += 10.0 : root.amount -= 10.0;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 128
        Layout.preferredHeight: 112
        text: "100"
        onClicked: {
            root.add ? root.amount += 100.0 : root.amount -= 100.0;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }

    MyNumPadButton {
        fontSize: 18
        Layout.preferredWidth: 168
        Layout.preferredHeight: 112
        text: "1000"
        onClicked: {
            root.add ? root.amount += 1000.0 : root.amount -= 1000.0;
            if(root.amount <= 0.0) root.amount = 0.0;
            amountUpdated(root.amount);
        }
    }
}