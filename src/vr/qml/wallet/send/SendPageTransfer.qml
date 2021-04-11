import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"

ColumnLayout {
	id: root
    spacing: 30

    property string txDialogText: ""
    property string amount: ""
    property string amount_type: "wow"
    property bool canSend: false

    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "How much would you like to send?"
    }

    function count(input, needle) {
        return input.split(".").length - 1;
    }

    RowLayout {
        spacing: 40

        ColumnLayout {
            Layout.preferredWidth: 320
            Layout.preferredHeight: 400

            MyNumPad {
                id: numPad
                compact: true
                onButtonPress: {
                    let periods = count(root.amount, ".");
                    if(periods == 1 && val === ".") return;
                    if(root.amount === "" && val === ".") return;
                    if(root.amount.length > 7) return;

                    root.amount += val;
                }
                onClearPress: {
                    root.amount = "";
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.Layout.preferredWidth
                color: "transparent"
            }
        }

        ColumnLayout {
            spacing: 14
            Layout.fillHeight: true
            Layout.preferredWidth: 280

            MyPushButton {
                id: wowButton
                opacity: enabled ? 1.0 : 0.4
                enabled: root.amount_type === "fiat"

                Layout.preferredWidth: 280
                Layout.preferredHeight: 108

                text: "Wownero"

                onClicked: {
                    root.amount_type = "wow"
                }
            }

            MyPushButton {
                id: fiatBtn
                opacity: enabled ? 1.0 : 0.4
                enabled: root.amount_type === "wow"

                Layout.preferredWidth: 280
                Layout.preferredHeight: 108

                text: "Fiat"

                onClicked: {
                    root.amount_type = "fiat"
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            spacing: 10
            Layout.fillHeight: true


            RowLayout {
                spacing: 30
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Rectangle {
                    color: "transparent"
                    Layout.preferredWidth: 192
                    Layout.preferredHeight: 48

                    MyText {
                        fontBold: true
                        text: "Amount:"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 68

                    MyText {
                        fontSize: 32
                        fontColor: Style.fontColorBright
                        text: {
                            let rtn = "";
                            if(root.amount === "") rtn += "0.0"
                            else if(root.amount_type === "wow") {
                                rtn += root.amount;
                            } else {
                                try {
                                    rtn += WowletVR.fiatToWow(root.amount);
                                } catch(err) {
                                    return "ERROR";
                                }
                            }

                            return rtn + " WOW";
                        }
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                    }
                }
            }

            RowLayout {
                spacing: 30
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Rectangle {
                    color: "transparent"
                    Layout.preferredWidth: 192
                    Layout.preferredHeight: 48

                    MyText {
                        fontBold: true
                        text: "Fiat:"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48

                    MyText {
                        id: fiatText
                        fontSize: 26
                        fontColor: Style.fontColorBright
                        text: {
                            let rtn = "";
                            if(root.amount === "") rtn += "0.0"
                            else if(root.amount_type === "fiat") {
                                rtn += root.amount;
                            } else {
                                try {
                                    rtn += WowletVR.wowToFiat(root.amount);
                                } catch(err) {
                                    return "ERROR";
                                }
                            }

                            return rtn + " " + appWindow.fiatSymbol
                        }
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                    }
                }
            }

            RowLayout {
                spacing: 30
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Rectangle {
                    color: "transparent"
                    Layout.preferredWidth: 192
                    Layout.preferredHeight: 48

                    MyText {
                        fontBold: true
                        text: "Destination:"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48

                    MyText {
                        text: destinationAddress.slice(0, 12) + "...";
                        fontColor: Style.fontColorBright
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            MyPushButton {
                id: sendButton

                Layout.preferredWidth: 420
                Layout.alignment: Qt.AlignRight

                text: "Create transaction"

                onClicked: {
                    if(amount === "") return;

                    let _amount = parseFloat(amount);
                    if(root.amount_type == "fiat") {
                        try {
                            _amount = WowletVR.fiatToWow(_amount);
                        } catch(err) {
                            messagePopup.showMessage("Error", "Could not convert fiat to wow.");
                            return;
                        }
                    }

                    if(amount <= 0) {
                        messagePopup.showMessage("Error", "Amount was zero.");
                        return;
                    }

                    WowletVR.onCreateTransaction(destinationAddress, _amount.toString(), "", false);  // no description
                    sendButton.enabled = false;
                }
            }
        }
    }

    function onPageCompleted() {
        root.amount = "";
        root.amount_type = "wow";
        root.txDialogText = "";
    }
    
    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Component.onCompleted: {
    }
}