// Copyright (c) 2014-2019, The Monero Project

import QtQuick 2.9
import QtGraphicalEffects 1.0

Item {
    // Use this component to color+opacity change images with transparency (svg/png)
    // Does not work in low graphics mode, use fontAwesome fallback option.

    id: root
    property string image: ""
    property string color: ""

    property alias svgMask: svgMask
    property alias imgMockColor: imgMockColor

    width: 0
    height: 0

    Image {
        id: svgMask
        source: root.image
        sourceSize.width: root.width
        sourceSize.height: root.height
        smooth: true
        mipmap: true
        visible: false
    }

    ColorOverlay {
        id: imgMockColor
        anchors.fill: root
        source: svgMask
        color: root.color
    }
}
