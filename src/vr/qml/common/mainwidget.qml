import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import ".."
import "../utilities_page"
import "../audio_page"
import "../motion_page"
import "../video_page"
import "../chaperone_page"
import "../steamvr_page"
import "../rotation_page"
import "../chaperone_page/chaperone_additional"

Rectangle {
    id: root
    color: "#1b2939"
    width: 1200
    height: 800

    property WalletDashboard WalletDashboard: WalletDashboard {
        stackView: walletView
    }

    property SteamVRPage steamVRPage: SteamVRPage {
        stackView: walletView
        visible: false
    }

    property ChaperonePage chaperonePage: ChaperonePage {
        stackView: walletView
        visible: false
    }

    property ChaperoneWarningsPage chaperoneWarningsPage: ChaperoneWarningsPage {
        stackView: walletView
        visible: false
    }

    property ChaperoneAdditionalPage chaperoneAdditionalPage: ChaperoneAdditionalPage{
        stackView: walletView
        visible: false
    }

    property PlayspacePage playspacePage: PlayspacePage {
        stackView: walletView
        visible: false
    }

    property MotionPage motionPage: MotionPage {
        stackView: walletView
        visible: false
    }

    property RotationPage  rotationPage: RotationPage {
        stackView: walletView
        visible: false
    }

    property FixFloorPage fixFloorPage: FixFloorPage {
        stackView: walletView
        visible: false
    }

    property StatisticsPage statisticsPage: StatisticsPage {
        stackView: walletView
        visible: false
    }

    property SettingsPage settingsPage: SettingsPage {
        stackView: walletView
        visible: false
    }

    property AudioPage audioPage: AudioPage {
        stackView: walletView
        visible: false
    }

    property UtilitiesPage utilitiesPage: UtilitiesPage {
        stackView: walletView
        visible: false
    }

    property VideoPage videoPage: VideoPage {
        stackView: walletView
        visible:false
    }

    StackView {
        id: walletView
        anchors.fill: parent

		pushEnter: Transition {
			PropertyAnimation {
				property: "x"
				from: walletView.width
				to: 0
				duration: 200
			}
		}
		pushExit: Transition {
			PropertyAnimation {
				property: "x"
				from: 0
				to: -walletView.width
				duration: 200
			}
		}
		popEnter: Transition {
			PropertyAnimation {
				property: "x"
				from: -walletView.width
				to: 0
				duration: 200
			}
		}
		popExit: Transition {
			PropertyAnimation {
				property: "x"
				from: 0
				to: walletView.width
				duration: 200
			}
		}

        initialItem: WalletDashboard
    }
}
