import QtQuick 2.7
import QtQuick.Controls 2.0

TextField {
	property int keyBoardUID: 0
    property string savedText: ""
    property bool passwordField: false
    
    id: myTextField
    echoMode: passwordField ? TextInput.Password : TextInput.Normal
    color: "#cccccc"
    text: ""
    font.pointSize: 20
    background: Button {
        hoverEnabled: true
        background: Rectangle {
            color: parent.hovered ? "#2c435d" : "#1b2939"
            border.color: "#cccccc"
            border.width: 2
        }
        onClicked: {
            myTextField.forceActiveFocus()
        }
    }
    onActiveFocusChanged: {
        if (activeFocus) {
            if (!OverlayController.desktopMode) {
                OverlayController.showKeyboard(text, keyBoardUID)
            } else {
                savedText = text
            }
        }
    }
    onEditingFinished: {
        if (OverlayController.desktopMode && savedText !== text) {
            myTextField.onInputEvent(text)
        }
    }
    function onInputEvent(input) {
        text = input
	}
}
