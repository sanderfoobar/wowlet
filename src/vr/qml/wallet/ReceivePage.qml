import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import "."
import "../common"

MyStackViewPage {
    id: root
    headerText: "Receive"

    content: ColumnLayout {
        Layout.fillWidth: true
        spacing: 30

        RowLayout {
            spacing: 30
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 20
                Layout.fillWidth: true

                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120

                    MyText {
                        width: parent.width
                        wrap: true
                        fontSize: 21
                        fontColor: Style.fontColorBright
                        text: "Give the following 4 digit PIN to the person that is sending you Wownero. PIN's are valid for 10 minutes and automatically renew."
                    }
                }

                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 220

                    ColumnLayout {
                        spacing: 20
                        MyText {
                            id: statusText
                            width: parent.width
                            visible: true
                            fontSize: 21
                            fontColor: Style.fontColorBright
                            text: "Generating PIN..."
                        }

                        Text {
                            id: pinText
                            text: "- - - -"
                            color: Style.fontColor
                            font.bold: true
                            font.pointSize: 60
                            leftPadding: 20
                            rightPadding: 20

                            Rectangle {
                                z: parent.z - 1
                                anchors.fill: parent
                                color: "black"
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            Rectangle {
                color: Style.fontColorDimmed
                width: 1
                Layout.fillHeight: true
            }

            ColumnLayout {
                Layout.fillWidth: true

                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    Layout.bottomMargin: 20

                    MyText {
                        width: parent.width
                        fontSize: 21
                        wrap: true
                        fontColor: Style.fontColorBright
                        text: "Alternatively, you may use one of the following methods to retreive your address."
                    }
                }

                MyPushButton {
                    id: viewAddress
                    text: "View address"
                    Layout.preferredHeight: 70
                    Layout.fillWidth: true

                    onClicked: {
                        let address = ctx.getAddress(0, 1);
                        messagePopup.showMessage("Your Wownero receiving address", address);
                    }
                }

                MyPushButton {
                    id: copyToClipboard
                    text: "Copy address to clipboard"
                    Layout.preferredHeight: 70
                    Layout.fillWidth: true

                    onClicked: {
                        let address = ctx.getAddress(0, 1);
                        WowletVR.setClipboard(address);
                        messagePopup.showMessage("Clipboard", "Address copied to clipboard.");
                    }
                }

                MyPushButton {
                    id: writeQRcode
                    text: "Save QR image"
                    Layout.preferredHeight: 70
                    Layout.fillWidth: true

                    onClicked: {

                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

    }

    function onPageCompleted() {
        console.log("onPageCompleted() ReceivePage")
        pinAskTimer.start();
        askReceivingPIN();
    }

    function resetPage() {
        pinAskTimer.stop();
        statusText.visible = true;
        statusText.text = "Generating PIN...";
        pinText.text = "- - - -";
    }

    onBackClicked: {
        resetPage();
    }

    Timer {
        id: pinAskTimer
        interval: 1000 * 5;
        running: false;
        repeat: true
        onTriggered: askReceivingPIN()
        triggeredOnStart: false
    }

    function askReceivingPIN() {
        ctx.onAskReceivingPIN();
    }

    Connections {
        target: ctx

        function onPinReceived(pin) {
            console.log("onPINReceived", pin);
            statusText.visible = false;
            pinText.text = pin[0] + " " + pin[1] + " " + pin[2] + " " + pin[3];
        }
    }

    Connections {
        target: OverlayController

        function onDashboardDeactivated() {
            console.log("onDashboardDeactivated()")
            pinAskTimer.stop();
        }

        function onDashboardActivated() {
            console.log("onDashboardActivated()")
            if(walletView.currentItem == root)
                pinAskTimer.start();
        }
    }
}
