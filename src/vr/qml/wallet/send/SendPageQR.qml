import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"


ColumnLayout {
	id: root
    spacing: 20

    property bool takingScreenshot: false
    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "Look at a QR code and press the button below to take a screenshot. Note: make sure to look at the center of the QR code. The parser works best with simple, straight-forward QR codes. When using more complex QR codes, make sure to properly fill your screen with the QR code itself (plus some margins)."
    }

    MyPushButton {
        id: continueButton
        text: "Take in-game screenshot"
        Layout.preferredWidth: 490
        opacity: takingScreenshot ? 0.0 : 1.0

        onClicked: {
            root.takingScreenshot = true;
            WowletVR.takeQRScreenshot();
        }
    }

    MyText {
        id: statusMessage
        visible: false
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "Status message."
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    Connections {
        target: WowletVR

        function onQrScreenshotSuccess(address) {
            root.takingScreenshot = false;
            console.log("onPinLookupReceived", address);
            if(!address.startsWith("wownero:")) {
                messagePopup.showMessage("Invalid QR code", "QR data did not start with \"wownero:\"");
                return;
            }

            if(sendStateView.currentView === sendStateView.qrPage) {
                sendStateController.destinationAddress = address.slice(8);
                sendStateView.state = "transferPage";
            }
        }

        function onQrScreenshotFailed(msg) {
            root.takingScreenshot = false;
            console.log("onQrScreenshotFailed", msg);
            messagePopup.showMessage("QR scan failure", msg)
            reset();
        }
    }

    function reset() {
        root.takingScreenshot = false;
    }

    function onPageCompleted(previousView){
        reset();
    }
}
