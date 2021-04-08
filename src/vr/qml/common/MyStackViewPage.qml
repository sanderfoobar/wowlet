import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import "."
import ".."

import wowlet.Wallet 1.0


Rectangle {
    id: root
    color: "transparent"
    width: 1600
    height: 800

    property StackView stackView
    property string headerText: "Header Title"
    property bool headerShowBackButton: true

    property string exitedColor: "transparent"

    signal backClicked();

    property Item header: ColumnLayout {
        RowLayout {
            Rectangle {
                color: "transparent"
                Layout.preferredWidth: headerBackButton.width + headerTitleContainer.width + 20
                Layout.preferredHeight: 70

                RowLayout {
                    anchors.fill: parent

                    Rectangle {
                        id: headerBackButton
                        visible: headerShowBackButton
                        color: "transparent"
                        Layout.preferredHeight: 50
                        Layout.preferredWidth: 50

                        ImageMask {
                            id: backIcon
                            anchors.fill: parent
                            image: "qrc:/backarrow"
                            color: Style.fontColorBright
                            width: 50
                            height: 50
                        }
                    }

                    Rectangle {
                        id: headerTitleContainer
                        color: "transparent"
                        Layout.preferredHeight: 50
                        Layout.preferredWidth: headerTitle.width

                        MyText {
                            id: headerTitle
                            text: headerText
                            font.pointSize: 26
                            anchors.verticalCenter: parent.verticalCenter
                            fontColor: Style.fontColorBright
                        }
                    }
                }

                MouseArea {
                    enabled: headerShowBackButton
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        parent.color = Style.btnEnteredColor
                        headerTitle.fontColor = Style.btnTextHoverColor
                        backIcon.color = Style.btnTextHoverColor
                    }
                    onExited: {
                        parent.color = root.exitedColor
                        headerTitle.fontColor = Style.fontColorBright
                        backIcon.color = Style.fontColorBright
                    }
                    onPressed: {
                        parent.color = Style.btnPressedColor
                        headerTitle.fontColor = Style.btnTextHoverColor
                        backIcon.color = Style.btnTextHoverColor
                    }
                    onClicked: {
                        stackView.pop();
                        backClicked();
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
            }


            Rectangle {
                Layout.preferredWidth: 720
                Layout.preferredHeight: 50
                color: "transparent"

                MyText{
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    fontSize: 14
                    fontBold: true
                    text: appWindow.balanceFormatted
                }
            }
        }

        Rectangle {
            color: Style.dividerColor
            height: 1
            Layout.topMargin: 10
            Layout.fillWidth: true
        }
    }

    property Item content: Frame {
        MyText {
            text: "Content"
        }
    }

    ColumnLayout {
        id: mainLayout
        spacing: 12
        anchors.fill: parent
    }

    Rectangle {
        id: bigRect
        color: "transparent"

        Layout.fillWidth: true
        Layout.preferredHeight: 64

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            anchors.leftMargin: 40
            anchors.rightMargin: 40

            color: Style.fontColorDimmed
            height: 1
        }

        RowLayout {
            spacing: 30
            anchors.verticalCenter: parent.verticalCenter

            Layout.preferredHeight: 64
            Layout.fillWidth: true

            MyText {
                Layout.leftMargin: 40

                fontSize: 14
                text: appWindow.statusText
                color: Style.fontColorDimmed
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: Style.fontColorDimmed
            }

            RowLayout {
                Layout.fillHeight: true
                spacing: 0

                MyText {
                    fontSize: 14
                    text: "Daemon: "
                    color: Style.fontColorDimmed
                }

                Image {
                    opacity: 0.8
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    source: {
                        if(typeof Wallet == 'undefined')
                            return "qrc:/status_disconnected";

                        if(appWindow.connectionStatus == Wallet.ConnectionStatus_Connected){
                            return "qrc:/status_connected";
                        } else if(appWindow.connectionStatus == Wallet.ConnectionStatus_Connecting) {
                            return "qrc:/status_lagging";
                        } else {
                            return "qrc:/status_disconnected";
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: Style.fontColorDimmed
            }

            RowLayout {
                Layout.fillHeight: true
                spacing: 0

                MyText {
                    fontSize: 14
                    text: "WS: "
                    color: Style.fontColorDimmed
                }

                Image {
                    opacity: 0.8
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    source: appWindow.wsConnected ? "qrc:/status_connected" : "qrc:/status_disconnected" 
                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: Style.fontColorDimmed
            }

            MyText {
                fontSize: 14
                text: "Balance: " + WowletVR.amountToFiat(appWindow.spendable);
                color: Style.fontColorDimmed
            }
        }
    }

    Component.onCompleted: {
        header.parent = mainLayout
        header.Layout.leftMargin = 40
        header.Layout.topMargin = 30
        header.Layout.rightMargin = 40
        content.parent = mainLayout
        content.Layout.fillHeight = true
        content.Layout.fillWidth = true

        content.Layout.topMargin = 10
        content.Layout.leftMargin = 40
        content.Layout.rightMargin = 40
        content.Layout.bottomMargin = 40

        bigRect.parent = mainLayout
    }
}
