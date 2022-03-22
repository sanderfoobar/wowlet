import QtQuick 2.7
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.3

Rectangle {
    id: root
    color: "#181725"
    anchors.fill: parent
    property variant buffer: []
    property int bufferMaxLength: 12
    // state:      0:idle 1:startup 2:syncing 3:mining
    signal startMining();
    signal stopMining();

    Component.onCompleted: {
        calcAvailableHeightConsoleLines();
    }

    onHeightChanged: {
        calcAvailableHeightConsoleLines();
    }

    function calcAvailableHeightConsoleLines() {
        var max_lines = parseInt(textContainer.height / 20);
        if(root.bufferMaxLength != max_lines && max_lines >= 4)
            root.bufferMaxLength = max_lines;
    }

//    width: 980
//    height: 480

    Column {
        FontLoader { id: comicMono; source: "qrc:/fonts/ComicMono.ttf" }
        FontLoader { id: comicMonoBold; source: "qrc:/fonts/ComicMono-Bold.ttf" }
    }

    ColumnLayout {
        width: parent.width
        height: parent.height
        spacing: 0

        RowLayout {
            spacing: 0
            Layout.fillWidth: true
            Layout.preferredHeight: 128

            Image {
                source: "qrc:/mining/topleft.png"
                Layout.preferredWidth: 435
                Layout.preferredHeight: 128
                smooth: false

                // top-left monitors
                ColumnLayout {
                    width: 102
                    height: 74

                    anchors.left: parent.left
                    anchors.leftMargin: 14
                    anchors.top: parent.top
                    anchors.topMargin: 42

                    Rectangle {
                        color: "transparent"
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            text: "Hashrate"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            font.pointSize: 14
                            font.family: comicMonoBold.name;
                            antialiasing: false
                            color: "#41FF00"
                        }
                    }

                    Rectangle {
                        color: "transparent"
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            id: hashRateText
                            text: "-"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            font.pointSize: 16
                            font.family: comicMono.name;
                            antialiasing: false
                            color: "#41FF00"
                        }
                    }
                }

                ColumnLayout {
                    width: 102
                    height: 74

                    anchors.left: parent.left
                    anchors.leftMargin: 142
                    anchors.top: parent.top
                    anchors.topMargin: 42

                    Rectangle {
                        color: "transparent"
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            text: "uptime"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            font.pointSize: 14
                            font.family: comicMonoBold.name;
                            antialiasing: false
                            color: "#41FF00"
                        }
                    }

                    Rectangle {
                        color: "transparent"
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Text {
                            id: miningUptime
                            text: "-"
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            font.pointSize: 12
                            font.family: comicMono.name;
                            antialiasing: false
                            color: "#41FF00"
                        }
                    }
                }

                AnimatedImage {
                    visible: mining.daemonMiningState !== 0
                    source: "qrc:/mining/mining.webp"
                    fillMode: Image.PreserveAspectCrop
                    width: 115
                    height: 86

                    anchors.left: parent.left
                    anchors.leftMargin: 263
                    anchors.top: parent.top
                    anchors.topMargin: 34
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 128
                spacing: 0

                Image {
                    source: "qrc:/mining/warning.png"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 15
                    fillMode: Image.TileHorizontally
                    smooth: false
                }

                Image {
                    source: "qrc:/mining/topright_bar.png"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 4
                    fillMode: Image.TileHorizontally
                    smooth: false
                }

                RowLayout {
                    spacing: 0
                    Layout.fillHeight: true
                    Layout.preferredHeight: 102

                    Image {
                        Layout.preferredWidth: 5
                        Layout.preferredHeight: parent.height
                        source: "qrc:/mining/topright_left.png"
                        smooth: false
                    }

                    Image {
                        Layout.fillWidth: true
                        Layout.preferredHeight: parent.height
                        source: "qrc:/mining/topright_middle.png"
                        fillMode: Image.TileHorizontally
                        smooth: false

                        RowLayout {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.topMargin: 12

                            height: 78
                            spacing: 16

                            ColumnLayout {
                                Layout.minimumWidth: 200
                                Layout.maximumWidth: 260
                                Layout.fillHeight: true

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "transparent"

                                    Text {
                                        text: "Block Height"
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 8
                                        font.pointSize: 20
                                        font.family: comicMonoBold.name;
                                        color: "#41FF00"
                                        antialiasing: false
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "transparent"

                                    Text {
                                        id: heightText
                                        text: "-"
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 8
                                        font.pointSize: 18
                                        font.family: comicMonoBold.name;
                                        color: "#41FF00"
                                        antialiasing: false
                                    }
                                }
                            }

                            ColumnLayout {
                                Layout.minimumWidth: 200
                                Layout.maximumWidth: 260
                                Layout.fillHeight: true

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "transparent"

                                    Text {
                                        text: "Sync Progress"
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 8
                                        font.pointSize: 20
                                        font.family: comicMonoBold.name;
                                        color: "#41FF00"
                                        antialiasing: false
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "transparent"

                                    Text {
                                        id: syncPctText
                                        text: "-"
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.horizontalCenter: parent.horizontalCenter

                                        font.pointSize: 18
                                        font.family: comicMonoBold.name;
                                        color: "#41FF00"
                                        antialiasing: false
                                    }
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                        }
                    }

                    Image {
                        Layout.preferredWidth: 5
                        Layout.preferredHeight: parent.height
                        source: "qrc:/mining/topright_right.png"
                        smooth: false
                    }
                }

                Item {
                    Layout.preferredHeight: 7  // 15 + 4 + 102 + 7 = 128
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            spacing: 0
            //Layout.preferredHeight: 128
            Layout.fillHeight: true
            Layout.fillWidth: true

            Image {
                Layout.preferredWidth: 6
                Layout.fillHeight: true
                source: "qrc:/mining/r_left.png"
                fillMode: Image.TileVertically
                smooth: false
            }

            Item {
                // text container
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    id: textContainer
                    color: "transparent"
                    height: parent.height - 20
                    width: parent.width - 32
                    clip: true

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        id: cons
                        anchors.margins: 4
                        anchors.fill: parent
                        text: "Miner is idle."
                        font.pointSize: 12
                        font.family: comicMono.name;
                        wrapMode: Text.WordWrap
                        color: "white"
                    }
                }
            }

            Image {
                Layout.preferredWidth: 6
                Layout.fillHeight: true
                source: "qrc:/mining/r_right.png"
                fillMode: Image.TileVertically
                smooth: false
            }
        }

        RowLayout {
            spacing: 0
            Layout.preferredHeight: 140
            Layout.fillWidth: true

            Image {
                Layout.preferredWidth: 306
                Layout.preferredHeight: 140
                source: "qrc:/mining/lowerleft.png"
                smooth: false

                 AnimatedImage {
                    source: mining.daemonMiningState === 0 ? "qrc:/assets/images/dog_sitting.gif" : "qrc:/assets/images/dog_running.gif"
                    width: 80
                    height: 60
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 22
                    anchors.left: parent.left
                    anchors.leftMargin: 22

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            showBubble();
                        }
                        onExited: {
                            hideBubble();
                        }
                    }
                }
            }

            Image {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                source: "qrc:/mining/lower_repeat.png"
                fillMode: Image.TileHorizontally
                smooth: false
            }

            Image {
                Layout.preferredWidth: 236
                Layout.preferredHeight: 140
                source: "qrc:/mining/bottom_center_console.png"
                smooth: false

                Rectangle {
                    // middle console clock
                    anchors.left: parent.left
                    anchors.leftMargin: 100
                    anchors.top: parent.top
                    anchors.topMargin: 8
                    color: "transparent"

                    width: 54
                    height: 16

                    Text {
                        id: clock
                        text: ""
                        antialiasing: false
                        font.pointSize: 9
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        font.family: comicMonoBold.name;
                        color: "#41FF00";

                        Component.onCompleted: {
                            root.setClock();
                        }
                    }
                }

                Image {
                    source: {
                        var imgs = ["qrc:/mining/amd.png", "qrc:/mining/intel.png"];
                        return imgs[Math.floor(Math.random()*imgs.length)];
                    }
                    width: 100
                    height: 100
                    fillMode: Image.Pad
                    smooth: false

                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter

                }
            }

            Image {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                source: "qrc:/mining/lower_repeat.png"
                fillMode: Image.TileHorizontally
                smooth: false
            }

            Image {
                Layout.preferredWidth: 308
                Layout.preferredHeight: 140
                source: "qrc:/mining/lowerright.png"
                smooth: false

                Rectangle {
                    // lower-right button container
                    color: "transparent"
                    width: 106
                    height: 100
                    anchors.right: parent.right
                    anchors.rightMargin: 11
                    anchors.top: parent.top
                    anchors.topMargin: 34

                    Image {
                        id: imgAxe
                        visible: mining.daemonMiningState === 0
                        source: "qrc:/mining/axe.png"
                        width: 73
                        height: 75
                        smooth: false
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    AnimatedImage {
                        visible: mining.daemonMiningState !== 0
                        source: "qrc:/mining/elmo.gif"
                        width: 106
                        height: 100
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter

                        Text {
                            id: stopMiningBtn
                            visible: true
                            text: "Stop Mining"
                            font.pointSize: 10
                            z: parent.z + 1
                            color: "black"
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: 28
                            font.family: comicMonoBold.name;
                            antialiasing: false
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        z: parent.z + 1
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            if(mining.daemonMiningState === 0) {
                                root.startMining();
                                root.calcAvailableHeightConsoleLines();
                            }
                            else
                                root.stopMining();
                        }
                        onEntered: {
                            imgAxe.height = 64
                            imgAxe.width = 64
                        }
                        onExited: {
                            imgAxe.height = 75
                            imgAxe.width = 73
                        }
                    }
                }
            }
        }
    }

    Image {
        id: bubble
        visible: false
        source: "qrc:/mining/bubble.png"
        width: 200
        height: 60
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 64
        anchors.left: parent.left
        anchors.leftMargin: 48

        Rectangle {
            anchors.top: parent.top
            anchors.topMargin: 6
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 26
            color: "transparent"
            width: 183
            z: parent.z + 1

            Text {
                id: bubbleText
                text: ""
                color: "black"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                //font.family: ComicMonoBold.name;
            }
        }
    }

    Timer {
        id: setClockTimer
        interval: 1000*60
        running: true
        repeat: true
        onTriggered: setClock()
    }

    Timer {
        id: dogBubbleTimer
        interval: 1000*30
        running: true
        repeat: true
        onTriggered: {
            if(Math.random() >= 0.5) return;
            if(bubble.visible) return;
            root.dogBubbleRemoval.stop();
            root.dogBubbleRemoval.start();

            var msg = root.bubbleMessage();

            bubbleText.text = msg;
            bubble.visible = true;
        }
    }

    Timer {
        id: dogBubbleRemoval
        interval: 2500
        running: false
        repeat: false
        onTriggered: bubble.visible = false;
    }

    function setClock() {
        var now = new Date();
        var hours = now.getHours();
        var minutes = ('0'+now.getMinutes()).slice(-2);
        clock.text = hours + ":" + minutes;
    }

    function resetComponents() {
        hashRateText.text = "-";
        miningUptime.text = "-";
        syncPctText.text = "-";
        heightText.text = "-";
    }

    function consoleAppend(line) {
        if(root.buffer.length >= root.bufferMaxLength)
            root.buffer.shift()
        root.buffer.push(line);

        cons.text = "";
        for(var i = 0; i != root.bufferMaxLength; i++) {
            if(root.buffer[i])
                cons.text += root.buffer[i] + "\n";
        }
    }

    Connections {
        target: mining
        function onDaemonOutput(line) {
            root.consoleAppend(line);
        }

        function onSyncStatus(from, to, pct) {
            syncPctText.text = pct + "%";
            heightText.text = from + "/" + to;
        }

        function onUptimeChanged(uptime) {
            miningUptime.text = uptime;
        }

        function onHashrate(hashrate) {
            hashRateText.text = hashrate;
        }
    }

    function showBubble() {
        if(bubble.visible) return;
        var msg = root.bubbleMessage();

        bubbleText.text = msg;
        bubble.visible = true;
    }

    function hideBubble() {
        bubble.visible = false;
    }

    function bubbleMessage() {
        var active = ["such work!", "mining WOW!", "woof woof!", "I am tired!", "mining :@", "weeeee", "blocks everywhere!", "wooohoo", "working, twerkin'", "looking4blocks", "mining blocks"];
        var inactive = ["doing nothing!", "ZzZZzzZ", "wen mining?!", "ETA mining?!", "zZZzzZZz", "omg so bored", "so bored!!", "much idle, many zZz", "lets go!", "i'm ready!"];
        var syncing = ["wen 1gbit", "syncin'", "zZzz", "ETA sync ready?!", "downloading blocks", "fetching blocks"];
        var msg = "";

        if(mining.daemonMiningState === 0) {
            return inactive[Math.floor(Math.random()*inactive.length)];
        } else if (mining.daemonMiningState === 2) {
            return syncing[Math.floor(Math.random()*syncing.length)];
        } else {
            return active[Math.floor(Math.random()*active.length)];
        }
    }
}
