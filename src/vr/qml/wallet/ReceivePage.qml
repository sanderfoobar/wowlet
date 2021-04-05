import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import "."
import "../common"

MyStackViewPage {
    headerText: "Receive"

    MyDialogOkPopup {
        id: chaperoneMessageDialog
        function showMessage(title, text) {
            dialogTitle = title
            dialogText = text
            open()
        }
    }

    MyDialogOkCancelPopup {
        id: chaperoneDeleteProfileDialog
        property int profileIndex: -1
        dialogTitle: "Delete Profile"
        dialogText: "Do you really want to delete this profile?"
        onClosed: {
            if (okClicked) {
                ChaperoneTabController.deleteChaperoneProfile(profileIndex)
            }
        }
    }

    MyDialogOkCancelPopup {
        id: chaperoneNewProfileDialog
        dialogTitle: "Create New Profile"
        dialogWidth: 800
        dialogHeight: 780
        dialogContentItem: ColumnLayout {
            RowLayout {
                Layout.topMargin: 16
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                MyText {
                    text: "Name: "
                }
                MyTextField {
                    id: chaperoneNewProfileName
                    keyBoardUID: 390
                    color: "#cccccc"
                    text: ""
                    Layout.fillWidth: true
                    font.pointSize: 20
                    function onInputEvent(input) {
                        chaperoneNewProfileName.text = input
                    }
                }
            }
            MyText {
                Layout.topMargin: 24
                text: "What to include:"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeGeometry
                Layout.leftMargin: 32
                text: "Chaperone Geometry"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeStyle
                Layout.leftMargin: 32
                text: "Chaperone Style"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeBoundsColor
                Layout.leftMargin: 32
                text: "Chaperone Color"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeVisibility
                Layout.leftMargin: 32
                text: "Visibility Setting"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeFadeDistance
                Layout.leftMargin: 32
                text: "Fade Distance Setting"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeCenterMarker
                Layout.leftMargin: 32
                text: "Center Marker Setting"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludePlaySpaceMarker
                Layout.leftMargin: 32
                text: "Play Space Marker Setting"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeFloorBoundsMarker
                Layout.leftMargin: 32
                text: "Floor Bounds Always On Setting"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeForceBounds
                Layout.leftMargin: 32
                text: "Force Bounds Setting"
            }
            MyToggleButton {
                id: chaperoneNewProfileIncludeProximityWarnings
                Layout.leftMargin: 32
                text: "Proximity Warning Settings"
            }
        }
        onClosed: {
            if (okClicked) {
                if (chaperoneNewProfileName.text == "") {
                    chaperoneMessageDialog.showMessage("Create New Profile", "ERROR: Empty profile name.")
                } else if (!chaperoneNewProfileIncludeGeometry.checked
                            && !chaperoneNewProfileIncludeVisibility.checked
                            && !chaperoneNewProfileIncludeFadeDistance.checked
                            && !chaperoneNewProfileIncludeCenterMarker.checked
                            && !chaperoneNewProfileIncludePlaySpaceMarker.checked
                            && !chaperoneNewProfileIncludeFloorBoundsMarker.checked
                            && !chaperoneNewProfileIncludeBoundsColor.checked
                            && !chaperoneNewProfileIncludeStyle.checked
                            && !chaperoneNewProfileIncludeForceBounds.checked
                            && !chaperoneNewProfileIncludeProximityWarnings.checked) {
                    chaperoneMessageDialog.showMessage("Create New Profile", "ERROR: Nothing included.")
                } else if ( Math.abs(MoveCenterTabController.offsetX) > 0.00000000001
                           || Math.abs(MoveCenterTabController.offsetY) > 0.00000000001
                           || Math.abs(MoveCenterTabController.offsetZ) > 0.00000000001
                           || MoveCenterTabController.rotation !== 0) {
                    chaperoneMessageDialog.showMessage("Create New Profile", "ERROR: Offsets not reset.")
                } else {
                    ChaperoneTabController.addChaperoneProfile(chaperoneNewProfileName.text,
                                                               chaperoneNewProfileIncludeGeometry.checked,
                                                               chaperoneNewProfileIncludeVisibility.checked,
                                                               chaperoneNewProfileIncludeFadeDistance.checked,
                                                               chaperoneNewProfileIncludeCenterMarker.checked,
                                                               chaperoneNewProfileIncludePlaySpaceMarker.checked,
                                                               chaperoneNewProfileIncludeFloorBoundsMarker.checked,
                                                               chaperoneNewProfileIncludeBoundsColor.checked,
                                                               chaperoneNewProfileIncludeStyle.checked,
                                                               chaperoneNewProfileIncludeForceBounds.checked,
                                                               chaperoneNewProfileIncludeProximityWarnings.checked)
                }

            }
        }
        function openPopup() {
            chaperoneNewProfileName.text = ""
            chaperoneNewProfileIncludeGeometry.checked = false
            chaperoneNewProfileIncludeVisibility.checked = false
            chaperoneNewProfileIncludeFadeDistance.checked = false
            chaperoneNewProfileIncludeCenterMarker.checked = false
            chaperoneNewProfileIncludePlaySpaceMarker.checked = false
            chaperoneNewProfileIncludeFloorBoundsMarker.checked = false
            chaperoneNewProfileIncludeBoundsColor.checked = false
            chaperoneNewProfileIncludeStyle.checked = false
            chaperoneNewProfileIncludeForceBounds.checked = false
            chaperoneNewProfileIncludeProximityWarnings.checked = false
            open()
        }
    }


    content: ColumnLayout {
        spacing: 30

        MyText {
            Layout.fillWidth: true
            wrap: true
            text: "Give the following 4 digit PIN to the person that is sending you Wownero. PIN's are valid for 5 minutes and automatically renew."
        }

        ColumnLayout {
            MyText {
                visible: false
                fontSize: 14
                text: "Currently generating PIN."
            }

            Text {
                visible: true
                text: "0 0 0 0"
                color: "#ffffff"
                font.bold: true
                font.pointSize: 40
                leftPadding: 20
                rightPadding: 20

                Rectangle {
                    z: parent.z - 1
                    anchors.fill: parent
                    color: "black"
                }
            }

            MyText {
                id: expireText
                visible: true
                fontSize: 14
                text: "Expires in 40 seconds."
            }


        }

        Rectangle {
            color: "#cccccc"
            height: 1
            Layout.topMargin: 10
            Layout.fillWidth: true
        }

        MyText {
            Layout.fillWidth: true
            wrap: true
            text: "Alternatively, you may use one of the following methods."
        }

        RowLayout {
            Layout.topMargin: 10

            MyPushButton {
                id: viewAddress
                text: "View address"
                Layout.preferredWidth: 360

                onClicked: {
                    MyResources.playFocusChangedSound()
                    walletView.push(chaperoneAdditionalPage)
                }
            }

            MyPushButton {
                id: copyToClipboard
                text: "Copy address to clipboard"
                Layout.preferredWidth: 540

                onClicked: {
                    MyResources.playFocusChangedSound()
                    walletView.push(chaperoneAdditionalPage)
                }
            }

            MyPushButton {
                id: writeQRcode
                text: "QR image"
                Layout.preferredWidth: 340

                onClicked: {
                    chaperoneNewProfileDialog.open();
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
    }
}
