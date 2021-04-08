import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import wowlet.NetworkType 1.0
import wowlet.WalletKeysFiles 1.0

import "."
import "common"

ColumnLayout {
	id: root
    width: 1600
    height: 800

	property var walletList: [];

	Layout.fillWidth: true
	Layout.fillHeight: true

	ColumnLayout {
		Layout.fillWidth: true
		Layout.preferredHeight: 128
		Layout.leftMargin: 40
		Layout.rightMargin: 40

		RowLayout {
            MyText {
                text: "Welcome to Wowlet VR"
                font.pointSize: 24
                Layout.leftMargin: 0
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
                    text: "Version beta (Qt " + qtRuntimeVersion + ")"
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

    Flow {
        id: flow
        spacing: 20
        clip: true

        property int itemHeight: 192
        property int maxRows: 6

        Layout.topMargin: 30
        Layout.fillWidth: true
        Layout.leftMargin: 40
    

		Repeater {
            id: recentList
            clip: true
            model: walletList

            delegate: Rectangle {
                id: item
                property var currentItem: walletList[index]
                property bool settings: currentItem.hasOwnProperty("settings")
                property bool create: currentItem.hasOwnProperty("create")
                property bool exit: currentItem.hasOwnProperty("exit")
                color: Style.btnMainMenuBackground
                height: 242
                width: 232

				Image {
					width: 158
					height: 158
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottom: parent.bottom
					anchors.bottomMargin: 10
					source: {
						if(settings) {
							return "qrc:/hypnotoad";
						} else if(create) {
							return "qrc:/wizard";
						} else if(exit) {
							return "qrc:/tutorial";
						} else {
							return "qrc:/chest";
						}
					}
				}

				Text {
                    id: btnText
					color: Style.btnTextColor
					text: {
						if(settings) {
							return "Settings";
						} else if(create) {
							return "Create wallet";
						} else if(exit) {
							return "Exit";
						} else {
							return currentItem['fileName'].replace(".keys", "");
						}
					}
					font.pointSize: 14
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.top: parent.top
					anchors.topMargin: 14
					anchors.leftMargin: 14
				}

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor

                    onEntered: {
                        parent.color = Style.btnPressedColor;
                        btnText.color = Style.btnTextHoverColor;
                    }
                    onExited: {
                        parent.color = Style.btnMainMenuBackground;
                        btnText.color = Style.btnTextColor;
                    }
                    onClicked: {
						if(settings) {
							mainView.push(settingsPage);
						} else if(create) {
							createWalletDialog.openPopup();
						} else if(exit) {
							OverlayController.exitApp();
						} else {
							appWindow.walletPath = currentItem['path'];
							ctx.onOpenWallet(currentItem['path'], "");
						}
                    }
                }
            }
        }
    }

	Item {
		Layout.fillWidth: true
		Layout.fillHeight: true
	}

    function onPageCompleted(previousView){
        console.log("list wallets");
    	
        root.walletList = [];
    	let wallets = [];
    	if(typeof ctx !== 'undefined')
     		wallets = ctx.listWallets();

    	let _walletList = [];

        for(var i = 0; i != wallets.length; i++) {
        	if(i == 9)  // draw 10 items at any time
        		break;
            _walletList.push(wallets[i]);
        }

        _walletList.push({"create": true});
        _walletList.push({"settings": true});
        _walletList.push({"exit": true});

        root.walletList = _walletList;
    }

    Connections {
    	target: appWindow
    	function onAboutClicked() {
    		mainView.push(aboutPage);
    	}

        function onRedraw() {
            console.log("hooray");
            onPageCompleted(1);
        }
    }
}
