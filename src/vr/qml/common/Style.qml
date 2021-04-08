pragma Singleton

import QtQuick 2.7

// These color codes are overriden by assets/themes.json, we are just 
// making sure the property names exist.

QtObject {
	id: root

    property string fontColor: "white"
    property string fontColorDimmed: "#cccccc"
    property string fontColorBright: "white"
    property string backgroundGradientStartColor: "#194f64"
    property string backgroundGradientStopColor: "#192e43"
    property string backgroundColor: "#1b2939"

    property string dividerColor: "#50ffffff"

    property string btnEnteredColor: "#aa3b689b"
    property string btnExitedColor: "#aa375c87"
    property string btnPressedColor: "#aa467dbb"
    property string btnTextColor: "white"
    property string btnTextHoverColor: "white"
    property string btnMainMenuBackground: "#aa3b689b"

    property string historyBackgroundColor: "#2c435d"
    property string historyBackgroundColorBright: "#406288"
    property string historyFontColorPlusAmount: "#00d304"
    property string historyFontColorMinAmount: "red"
}