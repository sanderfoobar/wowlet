import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"


ColumnLayout {
    id: root

    Layout.fillWidth: true
    property string _PINLookup: ""

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
                color: Style.fontColor
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
                    fontSize: 18
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
                    fontSize: 18
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
                        text: _PINLookup
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
        loadingContainer.visible = true;

        numPad.enabled = false;

        ctx.onLookupReceivingPIN(code);
    }

    function onPageCompleted(previousView) {
        reset();
    }

    function reset() {
        // reset state
        _PINLookup = "";

        idleContainer.visible = true;
        loadingContainer.visible = false;
        sendStateController.destinationAddress = "";

        numPad.enabled = true;
        numPad.reset();

        codeDisplay.text = "0 0 0 0";
    }

    Connections {
        target: ctx

        function onPinLookupReceived(address, pin) {
            console.log("onPinLookupReceived", address);

            if(pin === _PINLookup) {
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