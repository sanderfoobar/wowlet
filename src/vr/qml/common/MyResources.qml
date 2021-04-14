pragma Singleton
import QtQuick 2.7


QtObject {
	function playActivationSound() {
        OverlayController.playActivationSound()
	}
	function playFocusChangedSound() {
        OverlayController.playFocusChangedSound()
    }
}
