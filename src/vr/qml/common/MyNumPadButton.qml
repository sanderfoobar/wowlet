import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

Rectangle {
    id: root
    property string text: ""
    property int fontSize: 32
    property alias mouseArea: mouseArea
    property alias btnTextColor: btnText.fontColor

    signal clicked;

    Layout.preferredWidth: 106
    Layout.preferredHeight: 106
    color: Style.btnExitedColor

    MyText {
        id: btnText
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        fontColor: Style.btnTextColor
        text: root.text
        fontSize: root.fontSize
        fontBold: true
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            parent.color = Style.btnEnteredColor
            btnText.color = Style.btnTextHoverColor
        }
        onExited: {
            parent.color = Style.btnExitedColor
            btnText.color = Style.btnTextColor
        }
        onPressed: {
            parent.color = Style.btnPressedColor
            btnText.color = Style.btnTextHoverColor
        }
        onClicked: {
            root.clicked();
        }
    }
}