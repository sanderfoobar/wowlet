import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2

import "../common"
import "."

MyStackViewPage {
    id: walletDashboard
    width: 1600
    height: 800
    headerText: appWindow.walletTitle
    headerShowBackButton: false
    stackView: walletView

    content: Item {
        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                spacing: 32
                Layout.fillHeight: true
                Layout.fillWidth: true
                
                ColumnLayout {
                    Layout.preferredWidth: 250
                    Layout.maximumWidth: 250
                    Layout.fillHeight: true
                    spacing: 10
                    
                    MyPushButton {
                        id: steamVRButton
                        iconPath: "qrc:/send_icon"
                        activationSoundEnabled: false
                        text: "Send"
                        Layout.fillWidth: true
                        onClicked: {
                            //MyResources.playFocusChangedSound()
                            if(!appWindow.wsConnected) {
                                return messagePopup.showMessage("Please wait", "No connection to websocket server (yet).");
                            }

                            walletView.push(sendPage)
                            sendPage.onPageCompleted();
                        }
                    }
                    
                    MyPushButton {
                        id: chaperoneButton
                        iconPath: "qrc:/receive_icon"
                        activationSoundEnabled: false
                        text: "Receive"
                        Layout.fillWidth: true
                        onClicked: {
                            //MyResources.playFocusChangedSound()
                            if(!appWindow.wsConnected) {
                                return messagePopup.showMessage("Please wait", "No connection to websocket server (yet).");
                            }

                            walletView.push(receivePage)
                            receivePage.onPageCompleted();
                        }
                    }

                    MyPushButton {
                        id: rotationButton
                        iconPath: "qrc:/backarrow"
                        activationSoundEnabled: false
                        text: "Close"
                        Layout.fillWidth: true
                        onClicked: {
                            ctx.closeWallet(true, true);
                            mainView.pop();
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        color: "transparent"
                    }
                }

                HistoryTable {
                    id: historyView
                    Layout.fillHeight: true
                    Layout.fillWidth: true                    
                }
            }
        }
    }

    function onPageCompleted() {
        historyView.onPageCompleted();
    }
}