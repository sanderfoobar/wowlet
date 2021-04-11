import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "."

ColumnLayout {
    id: root
    enabled: true
    spacing: 16
    
    property double disabledOpacity: 0.4
    property bool compact: false

    signal resolvePressed();
    signal buttonPress(string val);
    signal clearPress();

    RowLayout {
        spacing: 18

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "1"
            onClicked: {
                buttonPress("1");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "2"
            onClicked: {
                buttonPress("2");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "3"
            onClicked: {
                buttonPress("3");
            }
        }
    }

    RowLayout {
        spacing: 18

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "4"
            onClicked: {
                buttonPress("4");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "5"
            onClicked: {
                buttonPress("5");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "6"
            onClicked: {
                buttonPress("6");
            }
        }
    }

    RowLayout {
        spacing: 18

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "7"
            onClicked: {
                buttonPress("7");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "8"
            onClicked: {
                buttonPress("8");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "9"
            onClicked: {
                buttonPress("9");
            }
        }
    }

    RowLayout {
        spacing: 18

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity
            text: "0"
            onClicked: {
                buttonPress("0");
            }
        }

        MyNumPadButton {
            visible: root.compact
            opacity: root.enabled ? 1 : disabledOpacity
            text: "."
            onClicked: {
                buttonPress(".");
            }
        }

        MyNumPadButton {
            opacity: root.enabled ? 1 : disabledOpacity

            Layout.preferredWidth: root.compact ? 106 : 230
            fontSize: 36
            text: root.compact ? "C" : "Clear"
            onClicked: {
                clearPress();
            }
        }
    }

    function reset() {

    }    
}
