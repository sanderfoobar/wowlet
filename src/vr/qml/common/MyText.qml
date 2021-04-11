import QtQuick 2.7
import QtQuick.Controls 2.0
import "."

Text {
	property bool wrap: false
	property int fontSize: 24
	property bool fontBold: false
	property string fontColor: Style.fontColor

	color: fontColor
	font.bold: fontBold
	font.pointSize: fontSize
	wrapMode: wrap ? Text.Wrap : Text.NoWrap
}
