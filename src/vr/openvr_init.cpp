// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.
// Copyright (c) OpenVR Advanced Settings

#include <string>
#include <openvr.h>
#include <QDebug>
#include <QMessageBox>

#include "openvr_init.h"
#include "utils/utils.h"
#include <openvr/src/vrcommon/vrpathregistry_public.h>
#include <openvr/src/vrcommon/pathtools_public.h>


namespace openvr_init
{
bool initializeProperly(const OpenVrInitializationType initType) {
    auto initializationType = vr::EVRApplicationType::VRApplication_Other;
    if (initType == OpenVrInitializationType::Overlay) {
        initializationType = vr::EVRApplicationType::VRApplication_Overlay;
    } else if (initType == OpenVrInitializationType::Utility) {
        initializationType = vr::EVRApplicationType::VRApplication_Utility;
    }

    auto initError = vr::VRInitError_None;
    vr::VR_Init(&initError, initializationType);
    if (initError != vr::VRInitError_None) {
        if (initError == vr::VRInitError_Init_HmdNotFound || initError == vr::VRInitError_Init_HmdNotFoundPresenceFailed) {
            QMessageBox::critical(nullptr, "Wowlet VR", "Could not find HMD!");
        }
        qCritical() << "Failed to initialize OpenVR: " << std::string(vr::VR_GetVRInitErrorAsEnglishDescription(initError)).c_str();
        return false;
    }

    qInfo() << "OpenVR initialized successfully.";
    return true;
}

bool initializeOpenVR(const OpenVrInitializationType initType)
{
    QString vr_pathreg_override = qgetenv("VR_PATHREG_OVERRIDE");
    if(!vr_pathreg_override.isEmpty()) {
        if(Utils::fileExists(vr_pathreg_override)) {
            qCritical() << "Filepath supplied in VR_PATHREG_OVERRIDE not found. Does this path exist?";
            return false;
        }
    }

    bool res = initializeProperly(initType);
    if(!res)
        return false;

    // Check whether OpenVR is too outdated
    if (!vr::VR_IsInterfaceVersionValid(vr::IVRSystem_Version)) {
        return reportVersionError(vr::IVRSystem_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRSettings_Version)) {
        return reportVersionError(vr::IVRSettings_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVROverlay_Version)) {
        return reportVersionError(vr::IVROverlay_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRApplications_Version)) {
        return reportVersionError(vr::IVRApplications_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRChaperone_Version)) {
        return reportVersionError(vr::IVRChaperone_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRChaperoneSetup_Version)) {
        return reportVersionError(vr::IVRChaperoneSetup_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRCompositor_Version)) {
        return reportVersionError(vr::IVRCompositor_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRNotifications_Version)) {
        return reportVersionError(vr::IVRNotifications_Version);
    } else if (!vr::VR_IsInterfaceVersionValid(vr::IVRInput_Version)) {
        return reportVersionError(vr::IVRInput_Version);
    }
    
    return true;
}

bool reportVersionError(const char* const interfaceAndVersion) {
    // The function call and error message was the same for all version checks.
    // Specific error messages are unlikely to be necessary since both the type
    // and version are in the string and will be output.
    auto msg = "OpenVR version is too outdated. Please update OpenVR: " + QString(interfaceAndVersion);
    QMessageBox::critical(nullptr, "Wowlet VR", msg);
    return false;
}

} // namespace openvr_init
