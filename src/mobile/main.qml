import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

import "."

import wowlet.Wallet 1.0
import wowlet.WalletManager 1.0

ApplicationWindow {
    visible: true
    id: appWindow
    width: 1080
    height: 2400
    color: "#2C3539"

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }

    Text {
        text: "Wowlet"
        color: "white"
        anchors.centerIn: parent
        font.pointSize: 62
    }
}
