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
     property var txModel
     property int txCount: 0

     property var txModelData: []
     property int sideMargin: 20

     ListModel { id: txListViewModel }

	ColumnLayout {		
		anchors.fill: parent
          spacing: 0

          MyText {
               visible: txCount == 0
               text: "No transactions to display."
               fontColor: Style.fontColorBright
          }

		ListView {
               id: listView
               visible: true

               Layout.fillWidth: true
               Layout.fillHeight: true
               spacing: 10
               model: txModel
               interactive: false

               delegate: Rectangle {
                    id: delegate
                    anchors.left: parent ? parent.left : undefined
                    anchors.right: parent ? parent.right : undefined
                    height: 54
                    color: Style.historyBackgroundColor

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
                         root.txCount = index;
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
                              color: Style.historyBackgroundColorBright

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
                                   fontSize: 22
                                   fontColor: Style.fontColorBright
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
                                   fontSize: 22
                                   text: description !== "" ? description : "..."
                                   fontColor: description !== "" ? Style.fontColorBright : Style.fontColorDimmed
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
                              color: Style.historyBackgroundColorBright

                              MyText {
                                   anchors.right: parent.right
                                   anchors.rightMargin: 10

                                   anchors.verticalCenter: parent.verticalCenter
                                   fontSize: 24
                                   fontBold: true
                                   text: amount
                                   fontColor: !isout ? Style.historyFontColorPlusAmount : Style.historyFontColorMinAmount
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
          var idx = txModel.index(x, y);
          return txModel.data(idx, 0);
     }

     function onPageCompleted() {
          if(currentWallet == null || typeof currentWallet.history === "undefined" ) return;
          root.txCount = 0;
          root.txModel = appWindow.currentWallet.historyModel;
          //root.model.sortRole = TransactionHistoryModel.TransactionBlockHeightRole;
          //root.model.sort(0, Qt.DescendingOrder);
     }
}