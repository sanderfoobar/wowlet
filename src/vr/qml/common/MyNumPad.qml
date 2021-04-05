import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "."

ColumnLayout {
    id: root
    enabled: true
    
    property double disabledOpacity: 0.4
    property var code: [];

    signal resolvePressed();
    signal codeUpdated(var pin_code);

    function onButtonPress(val) {
        code.push(val);

        if(code.length === 5)
            code = [val];

        codeUpdated(code);
    }

    function reset() {
        root.code = [];
    }

    RowLayout {
        spacing: 20

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "1"
            onClicked: {
                onButtonPress("1");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "2"
            onClicked: {
                onButtonPress("2");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "3"
            onClicked: {
                onButtonPress("3");
            }
        }
    }

    RowLayout {
        spacing: 20

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "4"
            onClicked: {
                onButtonPress("4");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "5"
            onClicked: {
                onButtonPress("5");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "6"
            onClicked: {
                onButtonPress("6");
            }
        }
    }

    RowLayout {
        spacing: 20

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "7"
            onClicked: {
                onButtonPress("7");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "8"
            onClicked: {
                onButtonPress("8");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "9"
            onClicked: {
                onButtonPress("9");
            }
        }
    }

    RowLayout {
        spacing: 20

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "0"
            onClicked: {
                onButtonPress("0");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity

            Layout.preferredWidth: 204
            fontSize: 16
            text: "Clear"
            onClicked: {
                root.code = [0,0,0,0];
                root.codeUpdated(root.code);
            }
        }
    }
}
