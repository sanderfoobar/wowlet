import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "../../common"

ColumnLayout {
	id: root
    spacing: 30

    property double amount: 0.0
    property bool canSend: false
    property bool isSending: false

    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        text: "How much would you like to send?"
    }

    MyNumPadSendAmount {
        id: myNumPadSendAmount
        Layout.fillWidth: true
        Layout.preferredHeight: 112
        Layout.maximumHeight: 112

        onAmountUpdated: {
            root.amount = amount;

            // @TODO: create tx validation here
            if(root.amount <= 0) {
                root.canSend = false;
            } else {
                root.canSend = true;
            }
        }
    }

    RowLayout {
        spacing: 30
        Layout.topMargin: 20
        Layout.fillHeight: true
        Layout.fillWidth: true

        ColumnLayout {
            spacing: 10
            Layout.fillHeight: true
            Layout.maximumWidth: parent.width / 2

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
                    Layout.preferredHeight: 48

                    MyText {
                        fontSize: 18
                        text: root.amount + " WOW"
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
                        fontSize: 18
                        text: "$853.20 USD"
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
                        text: "Wo3ige...YegEia2"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        Rectangle {
            color: "transparent"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    RowLayout {
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: 128

        MyPushButton {
            id: keyboardButton
            enabled: !isSending
            opacity: isSending ? 0.5 : 1.0
            Layout.preferredWidth: 700
            
            text: "Enter amount via virtual keyboard"
            
            onClicked: {
                OverlayController.showKeyboard(text, "1337")
            }
        }
        
        MyPushButton {
            id: sendButton
            opacity: root.canSend ? 1.0 : 0.5
            enabled: root.canSend

            Layout.preferredWidth: 420
            Layout.alignment: Qt.AlignRight

            text: "Create transaction"

            onClicked: {
                currentWallet.createTransactionAsync(addresses, paymentId, amountsxmr, mixinCount, priority);
            }
        }
    }

    Connections {
        target: OverlayController
        function onKeyBoardInputSignal(input, userValue) {
            if (userValue == "1337") {
                let val = parseFloat(input);
                myNumPadSendAmount.onAmountUpdated(val);
            }
        }
    }

    function onPageCompleted(previousView){

    }
}