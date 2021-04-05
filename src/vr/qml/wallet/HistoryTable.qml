import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import wowlet.Wallet 1.0
import wowlet.WalletManager 1.0
import wowlet.TransactionHistory 1.0
import wowlet.TransactionInfo 1.0
import wowlet.TransactionHistoryModel 1.0
import wowlet.TransactionHistoryProxyModel 1.0

import "../common"

Item {
     id: root
     property var modelx

     property var txModelData: []
     property int sideMargin: 20

     ListModel { id: txListViewModel }

	ColumnLayout {		
		anchors.fill: parent
          spacing: 0

		ListView {
               id: listView
               visible: true

               Layout.fillWidth: true
               Layout.fillHeight: true
               spacing: 10
               model: modelx
               interactive: false

               delegate: Rectangle {
               id: delegate
                    anchors.left: parent ? parent.left : undefined
                    anchors.right: parent ? parent.right : undefined
                    height: 54
                    color: "#2c435d"

                    property bool isout: false;
                    property string amount: "0";
                    property string description: "";
                    property string date: "2021-13-37";

                    property bool confirmed: false
                    property bool failed: false
                    property bool pending: false
                    property int confirmations: 0
                    property int confirmationsRequired: 0

                    Component.onCompleted: {
                         isout = getTxData(index, TransactionHistoryModel.TransactionIsOutRole);
                         amount = getTxData(index, TransactionHistoryModel.Amount);
                         description = getTxData(index, TransactionHistoryModel.Description);
                         date = getTxData(index, TransactionHistoryModel.Date);

                         failed = getTxData(index, TransactionHistoryModel.TransactionFailedRole);
                         pending = getTxData(index, TransactionHistoryModel.TransactionPendingRole);
                         confirmations = getTxData(index, TransactionHistoryModel.TransactionConfirmationsRole);
                         confirmationsRequired = getTxData(index, TransactionHistoryModel.TransactionConfirmationsRequiredRole);

                         confirmed = confirmations >= confirmationsRequired;
                    }

                    RowLayout {
                         spacing: 0
                         clip: true
                         height: parent.height
                         anchors.left: parent.left
                         anchors.right: parent.right

                         Rectangle {
                         	Layout.preferredWidth: 56
                         	Layout.fillHeight: true
                         	color: "#406288"

                         	Image {
                         		width: 32
                         		height: 32
                         		anchors.horizontalCenter: parent.horizontalCenter
                         		anchors.verticalCenter: parent.verticalCenter
                     			source: {
                                        if(failed) return "qrc:/assets/images/warning.png"
                                        else if(pending) return "qrc:/assets/images/unconfirmed.png"
                                        else if(!confirmed) return "qrc:/assets/images/clock1.png"

                                        else return "qrc:/assets/images/confirmed.png"
                                        //confirmed ? "qrc:/checkmark_icon" : "qrc:/expired_icon"
                                   }
                         	}
                         }

                         Rectangle {
                         	Layout.preferredWidth: 300
                         	Layout.leftMargin: 10
                         	Layout.rightMargin: 10
                         	Layout.fillHeight: true
                         	color: "transparent"

                         	MyText {
                                   // date
                         		anchors.verticalCenter: parent.verticalCenter
                         		fontSize: 12
                                   fontColor: "white"
                         		text: date

                         		Component.onCompleted: {
                         			parent.Layout.preferredWidth = width;
                         		}
                         	}
                         }

                         Rectangle {
                         	Layout.fillHeight: true
                         	Layout.leftMargin: 10
                         	color: "transparent"

                         	MyText {
                         		anchors.verticalCenter: parent.verticalCenter
                         		fontSize: 14
                         		text: description !== "" ? description : "..."
                                   fontColor: description !== "" ? "white" : "#cccccc"
                         		Component.onCompleted: {
                         			parent.Layout.preferredWidth = width;
                         		}
                         	}
                         }

                         Item {
                         	Layout.fillWidth: true
                         }

                         Rectangle {
                         	Layout.preferredWidth: 420
                         	Layout.fillHeight: true
                         	color: "#406288"

                         	MyText {
                         		anchors.right: parent.right
                         		anchors.rightMargin: 10

                         		anchors.verticalCenter: parent.verticalCenter
                         		fontSize: 14
                         		fontBold: true
                         		text: amount
                         		fontColor: !isout ? "#00d304" : "red"
                         	}
                         }
                     }
	        }
	    }

	    Item {
	    	Layout.fillHeight: true
	    }
	}

	Rectangle {
		z: parent.z - 1
		color: "transparent"
		anchors.fill: parent
	}

     function getTxData(x, y) {
          var idx = modelx.index(x, y);
          return modelx.data(idx, 0);
     }

     function updateTransactionsFromModel() {
          // This function copies the items of `appWindow.currentWallet.historyModel` to `root.txModelData`, as a list of javascript objects
          if(appWindow.currentWallet == null || typeof appWindow.currentWallet.history === "undefined" ) return;

          var _model = root.model;
          var total = 0
          var count = _model.rowCount()
          root.txModelData = [];
     }

     function onPageCompleted() {
          if(currentWallet == null || typeof currentWallet.history === "undefined" ) return;
          root.modelx = appWindow.currentWallet.historyModel;
          //root.model.sortRole = TransactionHistoryModel.TransactionBlockHeightRole;
          //root.model.sort(0, Qt.DescendingOrder);
     }
}