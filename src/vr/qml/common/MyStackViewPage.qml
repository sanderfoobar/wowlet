import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import "."
import ".."

import wowlet.Wallet 1.0


Rectangle {
    color: "#1b2939"
    width: 1600
    height: 800

    property StackView stackView
    property string headerText: "Header Title"
    property bool headerShowBackButton: true

    signal backClicked();

    property Item header: ColumnLayout {
        RowLayout {
            Button {
                id: headerBackButton
                Layout.preferredHeight: 50
                Layout.preferredWidth: 50
                hoverEnabled: true
                enabled: headerShowBackButton
                visible: headerShowBackButton
                contentItem: Image {
                    source: "qrc:/backarrow"
                    sourceSize.width: 50
                    sourceSize.height: 50
                    anchors.fill: parent
                }
                background: Rectangle {
                    opacity: parent.down ? 1.0 : (parent.activeFocus ? 0.5 : 0.0)
                    color: "#406288"
                    radius: 4
                    anchors.fill: parent
                }
                onHoveredChanged: {
                    if (hovered) {
                        forceActiveFocus()
                    } else {
                        focus = false
                    }
                }
                onClicked: {
					backClicked();
                    stackView.pop();
                }
            }
            MyText {
                id: headerTitle
                text: headerText
                font.pointSize: 30
                Layout.leftMargin: headerShowBackButton ? 32 : 0
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
            color: "#cccccc"
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
        id: testy66
        color: "transparent"

        Layout.fillWidth: true
        Layout.preferredHeight: 64

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            anchors.leftMargin: 40
            anchors.rightMargin: 40

            color: "#cccccc"
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
                color: "#cccccc"
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                color: "#cccccc"
            }

            RowLayout {
                Layout.fillHeight: true
                spacing: 0

                MyText {
                    fontSize: 14
                    text: "Daemon: "
                    color: "#cccccc"
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
                color: "#cccccc"
            }

            RowLayout {
                Layout.fillHeight: true
                spacing: 0

                MyText {
                    fontSize: 14
                    text: "WS: "
                    color: "#cccccc"
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
                color: "#cccccc"
            }

            MyText {
                fontSize: 14
                text: "Fiat: $0.05 USD"
                color: "#cccccc"
            }
        }
    }

    // Rectangle {
    //     z: 100
    //     color: "black"
    //     height: 64

    //     // anchors.bottom: parent.bottom
    //     // anchors.left: parent.left
    //     // anchors.right: parent.right

    //     Rectangle {
    //         anchors.top: parent.top
    //         anchors.left: parent.left
    //         anchors.right: parent.right

    //         anchors.leftMargin: 40
    //         anchors.rightMargin: 40

    //         color: "#cccccc"
    //         height: 1
    //     }

    //     RowLayout {
    //         spacing: 30
    //         anchors.left: parent.left
    //         anchors.leftMargin: 40
    //         anchors.rightMargin: 40
    //         anchors.verticalCenter: parent.verticalCenter

    //         MyText {
    //             fontSize: 14
    //             text: "Status: idle"
    //         }

    //         MyText {
    //             fontSize: 14
    //             text: "WS: OK"
    //         }

    //         MyText {
    //             fontSize: 14
    //             text: "Tor: OK"
    //         }

    //         MyText {
    //             fontSize: 14
    //             text: "Height: OK"
    //         }
    //     }
    // }

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

        testy66.parent = mainLayout
    }
}
