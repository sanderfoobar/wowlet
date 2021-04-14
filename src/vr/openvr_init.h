#pragma once

namespace openvr_init
{

    enum class OpenVrInitializationType
    {
        Overlay,
        Utility,
    };

    bool initializeProperly(OpenVrInitializationType initType);
    bool initializeOpenVR(OpenVrInitializationType initType );
    bool reportVersionError(const char* interfaceAndVersion);

} // namespace openvr_init
