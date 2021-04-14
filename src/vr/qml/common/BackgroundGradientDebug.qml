import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.2

import "."

// This file is used to debug the background gradient animation and can be ignored.

ColumnLayout {
    property int start_x: 0
    property int start_y: 64
    property int end_x: 1080
    property int end_y: 416
    
    visible: false
    width: parent.width
    height: parent.height

    MyText {
        fontColor: "red"
        text: "start_x: " + start_x
    }

    MySlider {
        from: 0
        to: 1600
        value: start_x
        Layout.fillWidth: true
        Layout.preferredHeight: 40

        onValueChanged: {
            appWindow.start_x = value;
        }
    }

    MyText {
        fontColor: "red"
        text: "start_y: " + start_y
    }

    MySlider {
        from: 0
        to: 1600
        value: start_y
        Layout.fillWidth: true
        Layout.preferredHeight: 40

        onValueChanged: {
            appWindow.start_y = value;
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    MyText {
        fontColor: "red"
        text: "end_x: " + end_x
    }

    MySlider {
        from: 0
        to: 1600
        value: end_x
        Layout.fillWidth: true
        Layout.preferredHeight: 40

        onValueChanged: {
            appWindow.end_x = value;
        }
    }

    MyText {
        fontColor: "red"
        text: "end_y: " + end_y
    }

    MySlider {
        from: 0
        to: 1600
        value: end_y
        Layout.fillWidth: true
        Layout.preferredHeight: 40

        onValueChanged: {
            appWindow.end_y = value;
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}