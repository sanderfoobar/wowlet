import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

import "../../common"
import "."

MyStackViewPage {
    id: sendStateController
    width: 1600
    headerText: "Send"

    property string destinationAddress: ""

    content: ColumnLayout {
        id: sendStateView
        property Item currentView
        property Item previousView

        property SendPagePIN pinPage: SendPagePIN {}
        property SendPageDashboard dashPage: SendPageDashboard {}
        property SendPageTransfer transferPage: SendPageTransfer {}
        property SendPageQR qrPage: SendPageQR {}

        state: ''

        onCurrentViewChanged: {
            if (previousView) {
               if (typeof previousView.onPageClosed === "function") {
                   previousView.onPageClosed();
               }
            }

            // if(previousView !== null && currentView.viewName === "wizardHome")
            //     wizardController.restart();

            if (currentView) {
                sendStack.replace(currentView)
                // Calls when view is opened
                if (typeof currentView.onPageCompleted === "function") {
                    currentView.onPageCompleted(previousView);
                }
            }

            previousView = currentView;

            // reset push direction
            // if(wizardController.wizardState == "wizardHome")
                // wizardController.wizardStackView.backTransition = false;
        }

        states: [
            State {
                name: "pinPage"
                PropertyChanges { target: sendStateView; currentView: sendStateView.pinPage }
            }, State {
                name: "dashPage"
                PropertyChanges { target: sendStateView; currentView: sendStateView.dashPage }
            }, State {
                name: "transferPage"
                PropertyChanges { target: sendStateView; currentView: sendStateView.transferPage }
            }, State {
                name: "qrPage"
                PropertyChanges { target: sendStateView; currentView: sendStateView.qrPage }
            }
        ]

        StackView {
            id: sendStack
            clip: true
            initialItem: sendStateView.dashPage
            
            Layout.fillWidth: true
            Layout.fillHeight: true

            delegate: StackViewDelegate {
                pushTransition: StackViewTransition {
                     PropertyAnimation {
                         target: enterItem
                         property: "x"
                         from: sendStack.backTransition ? -target.width : target.width
                         to: 0
                         duration: 300
                         easing.type: Easing.OutCubic
                     }

                     PropertyAnimation {
                         target: exitItem
                         property: "x"
                         from: 0
                         to: sendStack.backTransition ? target.width : -target.width
                         duration: 300
                         easing.type: Easing.OutCubic
                     }
                }
            }
        }
    }

    function onPageCompleted(previousView){
        sendStateView.state = "dashPage"
    }

    onBackClicked: {
        // top back button to send/receive menu, reset default states for sendViews
        sendStateView.pinPage.onPageCompleted();
        sendStateView.dashPage.onPageCompleted();
        sendStateView.transferPage.onPageCompleted();
    }
}
