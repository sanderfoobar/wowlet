import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import "../../common"


ColumnLayout {
	id: root
    spacing: 20

    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "Look at a QR code in VR and take a screenshot."
    }

    MyPushButton {
        id: continueButton
        text: "Take in-game screenshot"
        Layout.preferredWidth: 490

        onClicked: {
            // QR thingy
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

    function onPageCompleted(previousView){

    }
}