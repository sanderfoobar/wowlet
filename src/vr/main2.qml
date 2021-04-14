import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import "."
import "mock/Windows.js" as Windows
import "mock/Version.js" as Version
import "mock/NetworkType.js" as NetworkType
import "mock/Settings.js" as Settings
import "mock"
import "qml/common"

import "qml/."


Rectangle {
    width: 1600
    height: 800
    color: "red"

    property var currentWallet;
    property bool disconnected: currentWallet ? currentWallet.disconnected : false

    property WalletDashboard WalletDashboard: WalletDashboard {
        stackView: walletView
    }

    property SendPage sendPage: SendPage {
        stackView: walletView
        visible: false
    }

    property ReceivePage receivePage: ReceivePage {
        stackView: walletView
        visible: false
    }

    StackView {
        id: walletView
        anchors.fill: parent

        pushEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: walletView.width
                to: 0
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: -walletView.width
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "x"
                from: -walletView.width
                to: 0
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "x"
                from: 0
                to: walletView.width
                duration: 300
                easing.type: Easing.OutCubic
            }
        }

        initialItem: WalletDashboard
    }
}
