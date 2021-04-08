import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"

ColumnLayout {
	id: root
    spacing: 30

    property string txDialogText: ""
    property double amount: 0.0
    property bool canSend: false

    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "How much would you like to send?"
    }

    MyNumPadSendAmount {
        id: myNumPadSendAmount
        Layout.fillWidth: true
        Layout.preferredHeight: 112
        Layout.maximumHeight: 112

        onAmountUpdated: {
            root.amount = amount;
            fiatText.text = WowletVR.amountToFiat(root.amount);

            // @TODO: tx validation here
            if(root.amount <= 0) {
                root.canSend = false;
            } else if(root.amount > appWindow.spendable) {
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
                    Layout.preferredHeight: 68

                    MyText {
                        fontSize: 24
                        fontColor: Style.fontColorBright
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
                        id: fiatText
                        fontSize: 18
                        fontColor: Style.fontColorBright
                        text: "$0.00 USD"
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
        }

        Rectangle {
            color: "transparent"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    RowLayout {
        Layout.preferredWidth: parent.width
        Layout.preferredHeight: 128

        MyPushButton {
            id: keyboardButton
            Layout.preferredWidth: 700
            
            text: "Enter amount via virtual keyboard"
            
            onClicked: {
                OverlayController.showKeyboard(text, 1337)
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
                WowletVR.onCreateTransaction(destinationAddress, root.amount, "", false);  // no description
                sendButton.enabled = false;
            }
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Connections {
        target: OverlayController

        function onKeyBoardInputSignal(input, userValue) {
            if (userValue == 1337) {
                let val = parseFloat(input);
                if(val >= 0)
                    myNumPadSendAmount.onAmountUpdated(val);
            }
        }
    }

    Connections {
        target: ctx

        function onInitiateTransaction() {
            console.log("transactionStarted");

            mainView.opacity = 0.4;
            mainView.enabled = false;
            root.canSend = false;
            root.txDialogText = "Busy creating transaction. Hold on tight!";

            waitPopup.open();
        }

        function onCreateTransactionError(message) { // str
            console.log("onCreateTransactionError", message);
            waitPopup.close();

            mainView.opacity = 1.0;
            mainView.enabled = true;
            root.canSend = true;
            root.txDialogText = "";

            messagePopup.showMessage("Error creating tx", message);
        }

        function onCreateTransactionSuccess(tx, address) {  // PendingTransaction
            console.log("onCreateTransactionSuccess", address)
            root.txDialogText = "Submitting transaction to the Wownero network.";
        }

        function onTransactionCommitted(status, tx, txid) {  // bool,PendingTransaction,stringlist
            console.log("onTransactionCommitted", status);
            waitPopup.close();

            mainView.opacity = 1.0;
            mainView.enabled = true;
            root.canSend = true;
            root.txDialogText = "";

            walletView.pop();
        }
    }

    function onPageCompleted(previousView){

    }

    Popup {
        id: waitPopup

        implicitHeight: 100
        implicitWidth: 1200
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        ColumnLayout {
            anchors.fill: parent
            MyText {
                Layout.alignment: Qt.AlignHCenter
                fontColor: Style.fontColor
                text: root.txDialogText
            }
        }

        background: Rectangle {
            color: "black"
            opacity: 0.8
        }
    }

    Component.onCompleted: {
    }
}