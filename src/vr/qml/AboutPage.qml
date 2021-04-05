import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import "."
import "common"

ColumnLayout {
	id: root
    spacing: 30
    width: 1600
    height: 800

    RowLayout {
        Layout.topMargin: 40
        Layout.leftMargin: 40
        Layout.rightMargin: 40
        Layout.fillWidth: true
        Layout.preferredHeight: 200

        ColumnLayout {
            Layout.preferredWidth: 256
            Layout.maximumWidth: 256

            Image {
                Layout.preferredHeight: 256
                Layout.preferredWidth: 256

                source: "qrc:/illuminati"
            }


        }

        ColumnLayout {
            Layout.preferredWidth: 256

            MyText {
                Layout.leftMargin: 40
                Layout.rightMargin: 40
                Layout.fillWidth: true
                text: "Wowlet VR is an alternative QML interface for wowlet and was made over a 4 week period by eating lots of pizzas. It is the world's first cryptocurrency wallet with support for VR. Wowlet is Free and open-source (BSD-3) software and the source code can be studied on git.wownero.com/wowlet/wowlet"
                wrap: true
            }


        }
    }

    MyText {
        Layout.leftMargin: 40
        Layout.rightMargin: 40
        Layout.fillWidth: true
        text: "By \"dsc\" - April 2021. Shoutouts: OpenVR-AdvancedSettings, qvqc, Gatto, cisme, wowario, lza_menace, jwinterm, nioc, asymptotically, azy, selsta, kico, laura, thrmo, rottensox, solar, bl4sty, scoobybejesus (sorry if I forgot anyone!)"
        wrap: true
    }

    MyPushButton {
        text: "Back"
        Layout.leftMargin: 40
        Layout.preferredWidth: 220

        onClicked: {
            mainView.pop();
        }
    }

	Item {
		Layout.fillWidth: true
		Layout.fillHeight: true
	}

    function onPageCompleted(previousView){

    }
}

// OverlayController.exitApp();
