import QtQuick 2.7
import QtQuick.Controls 2.0


Text {
	property bool wrap: false
	property int fontSize: 16
	property bool fontBold: false
	property string fontColor: "#ffffff"

	color: fontColor
	font.bold: fontBold
	font.pointSize: fontSize
	wrapMode: wrap ? Text.WordWrap : Text.NoWrap
}
