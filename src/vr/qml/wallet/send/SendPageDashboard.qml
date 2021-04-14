import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "../../common"

ColumnLayout {
	id: root

    Layout.fillWidth: true

    MyText {
        Layout.fillWidth: true
        wrap: true
        fontColor: Style.fontColorBright
        text: "How to transfer Wownero?"
    }

    RowLayout {
        Layout.topMargin: 30
        Layout.fillWidth: true
        spacing: 30

        SendPageDashboardButton {
            displayText: "Send via PIN"
            onClicked: {
                sendStateView.state = "pinPage";
            }
        }

        SendPageDashboardButton {
            displayText: "Send via QR code"
            onClicked: {
                sendStateView.state = "qrPage";
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    function onPageCompleted(previousView){

    }
}