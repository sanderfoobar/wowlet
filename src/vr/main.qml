import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import "."

import "qml/common"
import "qml/."

import wowlet.Wallet 1.0
import wowlet.WalletManager 1.0

Rectangle {
    id: appWindow
    width: 1600
    height: 800
    color: "#1b2939"

    property var currentWallet;
    property bool disconnected: currentWallet ? currentWallet.disconnected : false
    property string walletTitle: "long wallet name"
    property string walletPath: ""
    property string statusText: "Idle"
    property string balanceFormatted: "Balance: 25928.9543 WOW"
    property bool wsConnected: false
    property int connectionStatus: Wallet.ConnectionStatus_Disconnected;

    property var balance: 0.0
    property var spendable: 0.0

    property DashboardPage dashboardPage: DashboardPage {
        visible: false
    }

    property AboutPage aboutPage: AboutPage {
        visible: false
    }

    property WalletPage walletPage: WalletPage {
        visible: false
    }

    MyDialogOkPopup {
        id: messagePopup
        function showMessage(title, text) {
            dialogTitle = title
            dialogText = text
            open()
        }
    }

    MyDialogOkCancelPopup {
        id: enterPasswordDialog
        dialogTitle: "Enter Wallet Password"
        dialogWidth: 700
        dialogHeight: 380
        
        dialogContentItem: ColumnLayout {
            RowLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                MyText {
                    text: "Password: "
                }

                MyTextField {
                    id: walletOpenPassword
                    keyBoardUID: 591
                    color: "#cccccc"
                    text: ""
                    Layout.fillWidth: true
                    font.pointSize: 20
                    function onInputEvent(input) {
                        walletOpenPassword.text = input
                    }
                }
            }
        }

        onClosed: {
            if (okClicked) {
                if(walletOpenPassword.text === "")
                    return messagePopup.showMessage("Password empty", "Please fill in a password.");

                ctx.onOpenWallet(appWindow.walletPath, walletOpenPassword.text);
            }
        }
        function openPopup() {
            open()
        }
    }

    MyDialogOkCancelPopup {
        id: createWalletDialog
        dialogTitle: "Create New Wallet"
        dialogWidth: 700
        dialogHeight: 440
        
        dialogContentItem: ColumnLayout {
            spacing: 10
            RowLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                MyText {
                    text: "Name: "
                }

                MyTextField {
                    id: newWalletName
                    keyBoardUID: 590
                    color: "#cccccc"
                    text: ""
                    Layout.fillWidth: true
                    font.pointSize: 20
                    function onInputEvent(input) {
                        newWalletName.text = input
                    }
                }
            }

            RowLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                MyText {
                    text: "Password: "
                }

                MyTextField {
                    id: newWalletPassword
                    keyBoardUID: 592
                    color: "#cccccc"
                    text: ""
                    Layout.fillWidth: true
                    font.pointSize: 20
                    function onInputEvent(input) {
                        newWalletPassword.text = input
                    }
                }
            }

            MyText {
                Layout.topMargin: 20
                Layout.leftMargin: 16
                fontSize: 16
                fontColor: "#cccccc"
                text: "The password field is optional."
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
        onClosed: {
            if (okClicked) {
                if(newWalletName.text === "")
                    return messagePopup.showMessage("Invalid name", "Please name the wallet.");

                ctx.createWalletWithoutSpecifyingSeed(newWalletName.text, newWalletPassword.text);
            }
        }
        function openPopup() {
            open()
        }
    }

    StackView {
        id: mainView
        anchors.fill: parent

        pushEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: mainView.width
                to: 0
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: -mainView.width
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: -mainView.width
                to: 0
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: mainView.width
                duration: 300
                easing.type: Easing.OutCubic
            }
        }

        initialItem: dashboardPage
    }

    Component.onCompleted: {
        dashboardPage.onPageCompleted();

        if(typeof ctx !== 'undefined') {
            ctx.initTor();
            ctx.initWS();
        }
    }

    Connections {
        target: ctx

        function onWsConnected() {
            console.log("onWsConnected")
            appWindow.wsConnected = true;
        }

        function onWsDisconnected() {
            console.log("onWsDisconnected")
            appWindow.wsConnected = false;
        }

        function onWalletOpened(wallet) {
            console.log("onWalletOpened()");

            appWindow.currentWallet = wallet;
            appWindow.walletTitle = ctx.walletName;
            mainView.push(appWindow.walletPage);
            appWindow.walletPage.onPageCompleted();

            appWindow.currentWallet.connectionStatusChanged.connect(onConnectionStatusChanged);
        }

        // function onWalletOpened(Wallet *wallet) {
            // currentWallet.moneySpent.connect(onWalletMoneySent)
            // currentWallet.moneyReceived.connect(onWalletMoneyReceived)
            // currentWallet.unconfirmedMoneyReceived.connect(onWalletUnconfirmedMoneyReceived)
            // currentWallet.transactionCreated.connect(onTransactionCreated)
            // currentWallet.connectionStatusChanged.connect(onWalletConnectionStatusChanged)
            // currentWallet.transactionCommitted.connect(onTransactionCommitted);

            // middlePanel.paymentClicked.connect(handlePayment);
        // }

        function onBlockchainSync(height, target) {
            let blocks = (target > height) ? (target - height) : "?";
            let heightText = "Blockchain sync: " + blocks + " blocks remaining";
            appWindow.statusText = heightText;
        }

        function onRefreshSync(height, target) {
            let blocks = (target >= height) ? (target - height) : "?";
            let heightText = "Wallet sync: " + blocks + " blocks remaining";
            appWindow.statusText = heightText;
        }

        function onWalletClosed() {
            console.log("onWalletClosed");

            appWindow.currentWallet.connectionStatusChanged.disconnect(onConnectionStatusChanged);

            appWindow.walletTitle = "";
            appWindow.balanceFormatted = "";
            appWindow.balance = 0.0;
            appWindow.spendable = 0.0;
            appWindow.connectionStatus = Wallet.ConnectionStatus_Disconnected;
        }

        function onBalanceUpdatedFormatted(fmt) {
            appWindow.balanceFormatted = fmt;
        }

        function onBalanceUpdated(balance, spendable) {
            appWindow.balance = balance;
            appWindow.spendable = spendable;
        }

        function onWalletOpenedError(err) {
            messagePopup.showMessage("Error", err);
        }

        function onWalletCreatedError(err) {
            messagePopup.showMessage("Error", err);
        }

        function onWalletCreated(wallet) {
            console.log("walletCreated");
        }

        function onSynchronized() {
            appWindow.statusText = "Synchronized";

            appWindow.onConnectionStatusChanged(Wallet.ConnectionStatus_Connected);
            console.log("onSynchronized");
        }

        function onWalletOpenPasswordNeeded(invalidPassword, path) { // bool, str
            enterPasswordDialog.openPopup();
        }

        function onInitiateTransaction() {
            console.log("transactionStarted");
        }

        function onCreateTransactionError(message) { // str
            console.log("transactionError", message);
        }

        function onCreateTransactionSuccess(tx, address) {  // PendingTransaction
            // auto-commit all tx's
            //m_ctx->currentWallet->commitTransactionAsync(tx);
            console.log("onCreateTransactionSuccess", address)
        }

        function onTransactionCommitted(status, tx, txid) {  // bool,PendingTransaction,stringlist
            console.log("onTransactionCommitted", status)
        }
    }

    function onConnectionStatusChanged(status) {
        console.log("onConnectionStatusChanged", status)
        appWindow.connectionStatus = status;
    }
}
