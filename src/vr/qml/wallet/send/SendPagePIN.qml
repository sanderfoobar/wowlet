import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"


ColumnLayout {
    id: root

    Layout.fillWidth: true
    property string pin: ""

    MyText {
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "Enter a 4 digit PIN and wait for it to resolve."
    }

    RowLayout {
        Layout.topMargin: 30
        Layout.fillWidth: true
        spacing: 40

        ColumnLayout {
            Layout.preferredWidth: 320
            Layout.preferredHeight: 400

            MyNumPad {
                id: numPad
                onButtonPress: {
                    root.pin += val;
                    if(root.pin.length === 4 && root.pin !== "0000") {
                        return addressLookup();
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.Layout.preferredWidth
                color: "transparent"
            }
        }

        ColumnLayout {
            Layout.preferredHeight: 400
            Layout.preferredWidth: 390

            Text {
                id: codeDisplay
                Layout.preferredWidth: 390
                visible: true
                text: (root.pin[0] || ".") + " " + (root.pin[1] || ".") + " " + (root.pin[2] || ".") + " " + (root.pin[3] || ".");
                color: Style.fontColor
                font.bold: true
                font.pointSize: 60
                leftPadding: 20
                rightPadding: 20

                Rectangle {
                    z: parent.z - 1
                    anchors.fill: parent
                    color: "black"
                }
            }

            Rectangle {
                color: "transparent"
                Layout.fillHeight: true
                Layout.preferredWidth: parent.Layout.preferredWidth
            }
        }

        Rectangle {
            color: Style.fontColorDimmed
            width: 1
            Layout.fillHeight: true
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            // Idle container
            ColumnLayout {
                id: idleContainer
                visible: true
                spacing: 30
                Layout.fillWidth: true

                MyText {
                    fontSize: 22
                    fontColor: Style.fontColorBright
                    text: "Waiting on input..."
                }
            }

            // Loading container
            ColumnLayout {
                id: loadingContainer
                visible: false
                spacing: 10
                Layout.fillWidth: true

                MyText {
                    fontSize: 22
                    fontColor: Style.fontColorBright
                    text: "Looking up address..."
                }

                RowLayout {
                    spacing: 30
                    Layout.topMargin: 20
                    Layout.fillWidth: true

                    MyText {
                        fontBold: true
                        fontColor: Style.fontColorBright
                        text: "Code:"
                    }

                    MyText {
                        text: root.pin
                    }
                }
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    function addressLookup() {
        console.log("addressLookup()");
        idleContainer.visible = false;
        loadingContainer.visible = true;

        numPad.enabled = false;

        try {
            ctx.onLookupReceivingPIN(root.pin);
        } catch(err){
            console.log("ctx.onLookupReceivingPIN() ignored")
        }
    }

    function onPageCompleted(previousView) {
        reset();
    }

    function reset() {
        console.log("SendPagePin reset()");
        // reset state
        root.pin = "";

        idleContainer.visible = true;
        loadingContainer.visible = false;
        sendStateController.destinationAddress = "";

        numPad.enabled = true;
        numPad.reset();
    }

    Connections {
        target: ctx

        function onPinLookupReceived(address, pin) {
            console.log("onPinLookupReceived", address);

            if(pin === root.pin) {
                sendStateController.destinationAddress = address;
                sendStateView.state = "transferPage";
            } else {
                console.log("PIN lookup received but we timed out already, disregard.")  // undefined behavior
            }
        }

        function onPinLookupErrorReceived() {
            console.log("onPinLookupErrorReceived");
            messagePopup.showMessage("Lookup failed", "Error getting address.")
            reset();
        }
    }
}