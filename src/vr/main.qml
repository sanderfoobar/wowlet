import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

import "."

import "qml/common"
import "qml/."

import wowlet.Wallet 1.0
import wowlet.WalletManager 1.0

Rectangle {
    id: appWindow
    width: 1600
    height: 800
    color: "transparent"

    property var themes: {}
    property string theme: "default"
    property string fiatSymbol: "USD"
    signal initTheme();

    // Components that have been dynamically created need to redraw
    // after theme change (such as Repeater{}, Flow{} items, etc) so 
    // that the changes propogate.
    signal redraw();

    // For gradient background
    property int start_x: 0
    property int start_y: 64
    property int end_x: 1080
    property int end_y: 416
    property double gradientTicks: 1.0;

    LinearGradient {
        anchors.fill: parent
        start: Qt.point(start_x, start_y)
        end: Qt.point(end_x, end_y)

        gradient: Gradient {
            GradientStop { position: 0.0; color: Style.backgroundGradientStartColor }
            GradientStop { position: 1.0; color: Style.backgroundGradientStopColor }
        }
    }

    Timer {
        // animates the gradient background a bit.
        id: gradientBackgroundTimer
        repeat: true
        interval: 10
        triggeredOnStart: true

        onTriggered: {
            appWindow.gradientTicks += 0.004;  // speed
            let newx = ((1080 - 200) * Math.sin(appWindow.gradientTicks) + 1080 + 200) / 2;
            appWindow.end_x = newx;
        }
    }

    property var currentWallet;
    property bool disconnected: currentWallet ? currentWallet.disconnected : false
    property string walletTitle: "placeholder"
    property string walletPath: ""
    property string statusText: "Idle"
    property string balanceFormatted: "Balance: 0.0 WOW"
    property bool wsConnected: false
    property int connectionStatus: Wallet.ConnectionStatus_Disconnected;
    signal aboutClicked();

    property var balance: 0.0
    property var spendable: 0.0

    property DashboardPage dashboardPage: DashboardPage {
        visible: false
    }

    property SettingsPage settingsPage: SettingsPage {
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
        dialogHeight: 280
        
        dialogContentItem: ColumnLayout {
            RowLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                MyText {
                    fontColor: Style.fontColorBright
                    text: "Password: "
                }

                MyTextField {
                    id: walletOpenPassword
                    keyBoardUID: 591
                    color: Style.fontColorDimmed
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
                    fontColor: Style.fontColorBright
                    text: "Name: "
                }

                MyTextField {
                    id: newWalletName
                    keyBoardUID: 590
                    color: Style.fontColorDimmed
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
                    fontColor: Style.fontColorBright
                    text: "Password: "
                }

                MyTextField {
                    id: newWalletPassword
                    keyBoardUID: 592
                    color: Style.fontColorDimmed
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
                fontSize: 24
                fontColor: Style.fontColorDimmed
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

    // About/credits button
    Rectangle {
        visible: mainView.currentItem == dashboardPage
        color: "transparent"
        width: 140
        height: 60
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        MyText {
            text: "Credits"
            fontSize: 16
            opacity: 0.3
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            fontColor: Style.fontColor
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                aboutClicked();
            }
        }
    }

    StackView {
        id: mainView
        visible: true
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

        // Start animating the background
        gradientBackgroundTimer.start();
        
        try {
            appWindow.themes = WowletVR.getThemes();
            appWindow.theme = WowletVR.getCurrentTheme();
        } catch(err) {
            // for debugging purposes - do not change color codes here, use themes.json instead.
            appWindow.themes = {
                "default": {
                    "fontColor": "white",
                    "fontColorDimmed": "#cccccc",
                    "fontColorBright": "white",
                    "backgroundGradientStartColor": "#225d73",
                    "backgroundGradientStopColor": "#192e43",
                }, 
                "wownero": {
                    "fontColor": "#bd93f9",
                    "fontColorDimmed": "#cccccc",
                    "fontColorBright": "#e5d3ff",
                    "backgroundGradientStartColor": "#383a59",
                    "backgroundGradientStopColor": "#282a36",
                }
            }
        }

        appWindow.changeTheme(appWindow.theme);
        appWindow.initTheme();
    }

    function changeTheme(theme) {
        console.log("changeTheme", theme);

        for (var key in appWindow.themes[theme]){
            let val = appWindow.themes[theme][key];
            if(Style.hasOwnProperty(key))
                Style[key] = val;
        }

        if(appWindow.theme != theme) {
            appWindow.theme = theme;
            try { WowletVR.setCurrentTheme(theme); }
            catch(err) {}
        }

        appWindow.redraw();
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
            appWindow.balance = WowletVR.cdiv(balance);
            appWindow.spendable = WowletVR.cdiv(spendable);
            console.log("onBalanceUpdated", appWindow.spendable);
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
            console.log("onCreateTransactionSuccess", address)
        }

        function onTransactionCommitted(status, tx, txid) {  // bool,PendingTransaction,stringlist
            console.log("onTransactionCommitted", status)
        }
    }

    Connections {
        target: OverlayController

        function onDashboardDeactivated() {
            gradientBackgroundTimer.stop();
        }

        function onDashboardActivated() {
            gradientBackgroundTimer.start();
        }
    }

    function onConnectionStatusChanged(status) {
        console.log("onConnectionStatusChanged", status)
        appWindow.connectionStatus = status;
    }
}
