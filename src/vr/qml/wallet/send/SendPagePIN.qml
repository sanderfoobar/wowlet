import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"


ColumnLayout {
    id: root

    Layout.fillWidth: true
    property string _PINLookup: ""
    property int lookupTimeoutSecs: 5;
    property int lookupTimeoutCounter: 2;

    Timer {
        id: lookupTimer
        interval: 1000;
        running: false;
        repeat: true;

        onTriggered: {
            lookupTimeoutCounter -= 1;

            if(lookupTimeoutCounter === 0) {
                stop();
                root.onLookupTimeout();
            }
        }
    }

    MyText {
        Layout.fillWidth: true
        wrap: true
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
                onCodeUpdated: {
                    let codeFmt = "";

                    for(var i = 0; i != 4; i++) {
                        if(pin_code[i] !== undefined) {
                            codeFmt += pin_code[i] + " ";
                        } else {
                            codeFmt += ". ";
                        }
                    }

                    // update display thingy
                    codeDisplay.text = codeFmt;

                    // lol Qt
                    codeFmt = codeFmt.replace(" ", "").replace(" ", "").replace(" ", "").replace(" ", "").replace(" ", "").trim();

                    if(pin_code.length === 4 && codeFmt != "0000")
                        addressLookup(codeFmt);
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
                text: "0 0 0 0"
                color: "#ffffff"
                font.bold: true
                font.pointSize: 40
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
            color: "#cccccc"
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
                    fontSize: 18
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
                    fontSize: 18
                    text: "Looking up address..."
                }

                RowLayout {
                    spacing: 30
                    Layout.topMargin: 20
                    Layout.fillWidth: true

                    MyText {
                        fontBold: true
                        text: "Code:"
                    }

                    MyText {
                        text: _PINLookup
                    }
                }

                RowLayout {
                    spacing: 30
                    Layout.fillWidth: true

                    MyText {
                        fontBold: true
                        text: "Timeout:"
                    }

                    MyText {
                        text: lookupTimeoutCounter + " seconds"
                    }
                }
            }

                // Image {
                //     visible: false
                //     id: loadingImage
                //     source: "qrc:/illuminati"
                //     sourceSize.width: 400
                //     sourceSize.height: 400
                // }

            ColumnLayout {
                id: foundContainer
                visible: false
                spacing: 30
                Layout.fillWidth: true

                RowLayout {
                    spacing: 20
                    Layout.fillWidth: true

                    MyText {
                        fontBold: true
                        text: "Address found:"
                    }

                    MyText {
                        text: "WW2xG...gKgrcC7"
                    }
                }

                MyPushButton {
                    id: continueButton
                    text: "Continue"
                    Layout.preferredWidth: 220

                    onClicked: {
                        //
                    }
                }
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    function addressLookup(code) {
        if(_PINLookup !== "")
            return;

        _PINLookup = code;

        idleContainer.visible = false;
        foundContainer.visible = false;
        loadingContainer.visible = true;

        numPad.enabled = false;

        lookupTimer.start();
    }

    function onLookupTimeout() {
        reset();
        messagePopup.showMessage("Lookup failed", "Error getting address.")
        sendStateView.state = "transferPage";
    }

    function onPageCompleted(previousView) {
        reset();
    }

    function onWSPINAdressReceived(pin_code, address) {
        // address received from websockets
        if(pin_code === _PINLookup) {
            sendStateController.destinationAddress = address;
            sendStateView.state = "transferPage";
        } else {
            console.log("PIN lookup received but we timed out already, disregard.")
        }
    }

    function reset() {
        // reset state
        _PINLookup = "";

        lookupTimer.stop();
        lookupTimeoutCounter = lookupTimeoutSecs;
        idleContainer.visible = true;
        foundContainer.visible = false;
        loadingContainer.visible = false;
        sendStateController.destinationAddress = "";

        numPad.enabled = true;
        numPad.reset();

        codeDisplay.text = "0 0 0 0";
    }
}