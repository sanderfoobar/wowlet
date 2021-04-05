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
    property string enteredColor: "#365473"
    property string exitedColor: "#2c435d"
    property string pressedColor: "#406288"

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
                    text: "Version 0.1 (Qt " + qtRuntimeVersion + ")"
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
                // inherited roles from walletKeysFilesModel:
                // index, fileName, modified, accessed, path, networktype, address
				// @TODO: maybe enforce networktype === 0 (mainnet)

                id: item
                property var currentItem: walletList[index]
                property bool about: currentItem.hasOwnProperty("about")
                property bool create: currentItem.hasOwnProperty("create")
                color: root.enteredColor
                height: 256
                width: 256

				Image {
					width: 182
					height: 182
					anchors.horizontalCenter: parent.horizontalCenter
					anchors.bottom: parent.bottom
					anchors.bottomMargin: 10
					source: {
						if(about) {
							return "qrc:/hypnotoad";
						} else if(create) {
							return "qrc:/wizard";
						} else {
							return "qrc:/chest";
						}
					}
				}

				Text {
					color: "white"
					text: {
						if(about) {
							return "About";
						} else if(create) {
							return "Create wallet";
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
                        parent.color = root.pressedColor
                    }
                    onExited: {
                        parent.color = root.enteredColor;
                    }
                    onClicked: {
						if(about) {
							mainView.push(aboutPage);
						} else if(create) {
							createWalletDialog.openPopup();
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
    	
    	let wallets = [];
    	if(typeof ctx !== 'undefined')
     		wallets = ctx.listWallets();

    	let _walletList = [];

        for(var i = 0; i != wallets.length; i++) {
        	if(i == 8)  // draw 10 items at any time
        		break;
            _walletList.push(wallets[i]);
        }

        _walletList.push({"create": true});
        _walletList.push({"about": true});

        root.walletList = _walletList;
    }
}

// OverlayController.exitApp();
