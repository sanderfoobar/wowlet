import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import "."
import "common"

ColumnLayout {
	id: root
    property bool needsRestart: false

    spacing: 30
    Layout.fillHeight: true
    Layout.fillWidth: true

    ColumnLayout {
        spacing: 30
        Layout.topMargin: 40
        Layout.fillWidth: true
        Layout.leftMargin: 40
        Layout.rightMargin: 40

        MyText {
            Layout.fillWidth: true
            fontSize: 26
            text: "Settings"
            wrap: true
        }

        RowLayout {
            spacing: 30

            MyText {
                text: "Theme"
            }

            MyComboBox {
                id: themeCombo
                Layout.preferredHeight: 60
                Layout.preferredWidth: 378

                displayText: "-"

                model: [""]

                delegate: ItemDelegate {
                    width: parent.width
                    text: modelData.text
                    hoverEnabled: true
                    contentItem: MyText {
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        text: parent.text
                        color: parent.enabled ? Style.fontColor : Style.fontColorDimmed
                    }
                    background: Rectangle {
                        color: parent.pressed ? "#406288" : (parent.hovered ? "#365473" : "#2c435d")
                    }
                }

                onCurrentIndexChanged: {
                    displayText = themeCombo.model[currentIndex]["text"];
                    if(displayText !== "" && displayText !== appWindow.theme) {
                        appWindow.changeTheme(displayText);
                        needsRestart = true;
                    }
                }
            }
        }
    }

    Component.onCompleted: {
    }

    MyDialogOkPopup {
        id: restartPopup
        dialogTitle: "Restart required"
        dialogText: "WowletVR needs a restart to load the theme. Exiting."    
        onClosed: {
            OverlayController.exitApp();
        }
    }

    MyPushButton {
        text: "Back"
        Layout.leftMargin: 40
        Layout.preferredWidth: 220

        onClicked: {
            if(root.needsRestart) {
                restartPopup.open();
            } else {
                mainView.pop();
            }
        }
    }

	Item {
		Layout.fillWidth: true
		Layout.fillHeight: true
	}

    function onPageCompleted(previousView){

    }

    Connections {
        target: appWindow

        function onInitTheme() {
            // populate combobox
            let themeComboBoxItems = [{ value: 0, text: ""}];
            for (let _theme in appWindow.themes){
                if (!appWindow.themes.hasOwnProperty(_theme))
                    continue;
                themeComboBoxItems.push({ value: 0, text: _theme });
            }
            themeCombo.model = themeComboBoxItems;
            themeCombo.displayText = appWindow.theme;
        }
    }
}

// OverlayController.exitApp();
