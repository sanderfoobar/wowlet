import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "../../common"

Rectangle {
    id: root
	color: Style.btnExitedColor
	Layout.fillWidth: true
	Layout.fillHeight: true

    property string displayText: ""

    signal clicked();

    MyText{
        id: btnText
        text: displayText
        fontSize: 24
        fontColor: Style.btnTextColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            parent.color = Style.btnEnteredColor;
            btnText.fontColor = Style.btnTextHoverColor;
        }
        onExited: {
            parent.color = Style.btnExitedColor;
            btnText.fontColor = Style.btnTextColor;
        }
        onPressed: {
            parent.color = Style.btnPressedColor;
            btnText.fontColor = Style.btnTextHoverColor;
        }
        onClicked: {
            root.clicked();
        }
    }
}