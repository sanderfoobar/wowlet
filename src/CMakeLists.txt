set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# pthread
find_package(Threads REQUIRED)

if(QML)
    # PNG
    find_package(ZLIB REQUIRED)
    find_package(PNG REQUIRED)
endif()

# Compile these source files (.h/.cpp)
file(GLOB SOURCE_FILES
        "*.h"
        "*.cpp"
        "utils/*.h"
        "utils/*.cpp"
        "libwalletqt/*.h"
        "libwalletqt/*.cpp"
        "daemon/*.h"
        "daemon/*.cpp"
        "model/*.h"
        "model/*.cpp"
        "qt/*.h"
        "qt/*.cpp"
        "ui/qdarkstyle/style.qrc"
        "ui/BreezeStyleSheets/breeze.qrc"
        "widgets/*.h"
        "widgets/*.cpp"
        "wizard/*.h"
        "wizard/*.cpp"
        "wallet/*.h"
        "wallet/*.cpp"
        "qrcode/*.h"
        "qrcode/*.cpp"
        "dialog/*.h"
        "dialog/*.cpp"
        )

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Network Svg Xml WebSockets Quick QuickWidgets Qml QuickControls2 QuickCompiler QmlImportScanner Multimedia)

if(OPENVR)
    # include some extra files
    qt5_add_resources(RESOURCES vr/qml.qrc)
    file(GLOB SOURCE_FILES_QML
            "vr/*.h"
            "vr/*.cpp"
            "vr/utils/*.h"
            "vr/utils/*.cpp"
            )
    list(APPEND SOURCE_FILES ${SOURCE_FILES_QML})
endif()

if(ANDROID OR ANDROID_DEBUG)
    qt5_add_resources(RESOURCES mobile/qml.qrc)
    file(GLOB SOURCE_FILES_QML
            "mobile/*.h"
            "mobile/*.cpp"
            )
    list(APPEND SOURCE_FILES ${SOURCE_FILES_QML})
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-deprecated-declarations")  # @TODO: removeme

add_subdirectory(libwalletqt)
add_subdirectory(model)
add_subdirectory(utils)
add_subdirectory(openpgp)

if(WITH_SCANNER)
    add_subdirectory(QR-Code-scanner)
endif()

qt5_add_resources(RESOURCES assets.qrc)

if(TOR_BIN)
    if(APPLE)
        set(ASSETS_TOR "assets_tor_macos.qrc")
    else()
        set(ASSETS_TOR "assets_tor.qrc")
    endif()
endif()

set(EXECUTABLE_FLAG)
if(MINGW)
    set(EXECUTABLE_FLAG WIN32)

    set(ICON ${PROJECT_SOURCE_DIR}/src/assets/images/appicons/appicon.ico)
    set(ICON_RC ${CMAKE_CURRENT_BINARY_DIR}/icon.rc)
    set(ICON_RES ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
    file(WRITE ${ICON_RC} "IDI_ICON1 ICON DISCARDABLE \"${ICON}\"")
    find_program(Qt5_WINDRES_EXECUTABLE NAMES windres x86_64-w64-mingw32-windres REQUIRED CMAKE_FIND_ROOT_PATH_BOTH)
    add_custom_command(OUTPUT ${ICON_RES} COMMAND ${Qt5_WINDRES_EXECUTABLE} ${ICON_RC} ${ICON_RES} MAIN_DEPENDENCY ${ICON_RC})
    list(APPEND RESOURCES ${ICON_RES})
endif()

# mac os bundle
if(APPLE)
    set(ICON ${PROJECT_SOURCE_DIR}/src/assets/images/appicons/appicon.icns)
    set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND RESOURCES ${ICON})
endif()

if(NOT ANDROID)
    add_executable(wowlet ${EXECUTABLE_FLAG} main.cpp
            ${SOURCE_FILES}
            ${RESOURCES}
            ${ASSETS_TOR}
            )
else()
    add_library(wowlet SHARED ${SOURCE_FILES} ${RESOURCES})
    set_target_properties(wowlet PROPERTIES COMPILE_DEFINITIONS "ANDROID")
endif()

# mac os bundle
set_target_properties(wowlet PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/utils/Info.plist"
        LINK_FLAGS_RELEASE -s
)

set_property(TARGET wowlet PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(wowlet PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(wowlet PUBLIC ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

file(GLOB_RECURSE SRC_SOURCES *.cpp)
file(GLOB_RECURSE SRC_HEADERS *.h)

target_include_directories(wowlet PUBLIC
        ${CMAKE_BINARY_DIR}/src/wowlet_autogen/include
        ${CMAKE_SOURCE_DIR}/wownero/include
        ${CMAKE_SOURCE_DIR}/wownero/src
        ${CMAKE_SOURCE_DIR}/wownero/external/easylogging++
        ${CMAKE_SOURCE_DIR}/wownero/contrib/epee/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/daemon
        ${CMAKE_CURRENT_SOURCE_DIR}/libwalletqt
        ${CMAKE_CURRENT_SOURCE_DIR}/model
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/tor
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode
        ${X11_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5WebSockets_INCLUDE_DIRS}
        )

if(OPENVR)
    target_include_directories(wowlet PUBLIC ${CMAKE_SOURCE_DIR}/contrib/)
endif()

if(DONATE_BEG)
    target_compile_definitions(wowlet PRIVATE DONATE_BEG=1)
endif()

if(TOR_BIN)
    target_compile_definitions(wowlet PRIVATE HAS_TOR_BIN=1)
endif()

if(ANDROID)
    target_compile_definitions(wowlet PRIVATE HAS_ANDROID=1)
endif()

if(ANDROID_DEBUG)
    target_compile_definitions(wowlet PRIVATE HAS_ANDROID_DEBUG=1)
endif()

if(OPENVR)
    target_compile_definitions(wowlet PRIVATE HAS_OPENVR=1)
    target_compile_definitions(wowlet PUBLIC VR_API_PUBLIC)
endif()

if(HAVE_SYS_PRCTL_H)
    target_compile_definitions(wowlet PRIVATE HAVE_SYS_PRCTL_H=1)
endif()

if(STATIC)
    target_compile_definitions(wowlet PRIVATE STATIC=1)
endif()

if(STATIC)
    target_compile_definitions(wowlet PRIVATE STATIC=1)
endif()

if("$ENV{DRONE}" STREQUAL "true")
    target_compile_definitions(wowlet PRIVATE DRONE=1)
endif()

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(wowlet PRIVATE QT_NO_DEBUG=1)
endif()

add_definitions(${QT_DEFINITIONS})
if(NOT "${CMAKE_BUILD_TYPE}" EQUAL "Debug")
    add_definitions(-DQT_NO_DEBUG)
endif()

target_compile_definitions(wowlet PUBLIC VR_API_PUBLIC)

qt5_import_qml_plugins(${PROJECT_NAME})

target_compile_definitions(wowlet
        PUBLIC
        ${Qt5Core_DEFINITIONS}
        ${Qt5Widgets_DEFINITIONS}
        ${Qt5Gui_DEFINITIONS}
        ${Qt5Network_DEFINITIONS}
        ${Qt5Svg_DEFINITIONS}
        ${Qt5Xml_DEFINITIONS}
        ${Qt5WebSockets_DEFINITIONS}
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(UNIX AND NOT APPLE)
    # https://stackoverflow.com/questions/57766620/cmake-add-library-doesnt-initialize-static-global-variable
    # so that contrib/monero-seed/src/gf_elem.cpp properly initializes. A better solution is welcome.
    target_link_libraries(wowlet PUBLIC -Wl,--whole-archive monero-seed::monero-seed -Wl,--no-whole-archive)
else()
    target_link_libraries(wowlet PUBLIC monero-seed::monero-seed)
endif()

if(ANDROID)
    # yolo some hardcoded paths
    target_include_directories(wowlet PUBLIC
            /opt/android/prefix/include/QtAndroidExtras/
            )
endif()

# Link Wownero core libraries
target_link_libraries(wowlet PUBLIC
        epee
        wallet_api
        easylogging
        ${Boost_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CMAKE_DL_LIBS}
        ${EXTRA_LIBRARIES})

# Link Qt libraries
target_link_libraries(wowlet PUBLIC
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Network
        Qt5::Svg
        Qt5::QSvgPlugin
        Qt5::QSvgIconPlugin
        Qt5::Xml
        Qt5::WebSockets
        Qt5::Quick
        Qt5::Qml
        Qt5::QuickControls2
        Qt5::QuickWidgets)

if(ANDROID)
    # yolo some hardcoded paths
    target_link_libraries(wowlet PUBLIC
            /opt/android/prefix/lib/libQt5QuickTemplates2_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Quick_arm64-v8a.so
            /opt/android/prefix/lib/libQt5QmlModels_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Qml_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Svg_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Widgets_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Gui_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Xml_arm64-v8a.so
            /opt/android/prefix/lib/libQt5XmlPatterns_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Network_arm64-v8a.so
            /opt/android/prefix/lib/libQt5Core_arm64-v8a.so
            /opt/android/prefix/lib/libQt5VirtualKeyboard_arm64-v8a.so
            /opt/android/prefix/lib/libQt5AndroidExtras_arm64-v8a.so
            /opt/android/prefix/plugins/bearer/libplugins_bearer_qandroidbearer_arm64-v8a.so
            GLESv2
            log
            z
            jnigraphics
            android
            EGL
            c++_shared
            )
endif()

# Link random other stuff
target_link_libraries(wowlet PUBLIC
        ${ICU_LIBRARIES}
        openpgp
        Threads::Threads
        ${QRENCODE_LIBRARY}
        )

# Link scanner
if(WITH_SCANNER)
    target_link_libraries(wowlet PUBLIC qrdecoder qrscanner)
    if(LINUX AND NOT ANDROID)
        target_link_libraries(wowlet PUBLIC
                jpeg
                v4l2
                v4lconvert
                rt
                )
    endif()
endif()

# Link OpenVR
if(OPENVR)
    if(MINGW)
        target_link_libraries(wowlet PUBLIC
                openvr_api64
                gcc stdc++ winpthread ssp glu32 opengl32 glmf32 -dynamic)
    else()
        target_link_libraries(wowlet PUBLIC openvr_api)
    endif()
endif()

if(APPLE)
    target_link_libraries(wowlet PUBLIC
            KDMacTouchBar
            )
    target_include_directories(wowlet
            PUBLIC ../contrib/KDMacTouchBar)
endif()

if(NOT APPLE)
    target_link_libraries(wowlet PUBLIC
            Qt5::QSvgIconPlugin
            Qt5::QSvgPlugin
    )
endif()

if(STATIC)
    target_link_libraries(wowlet PUBLIC
            Qt5::QSvgIconPlugin
            Qt5::QSvgPlugin)
    if(UNIX AND NOT APPLE)
	    target_link_libraries(wowlet PUBLIC
            	Qt5::QXcbIntegrationPlugin)
    endif()
endif()

if(X11_FOUND)
    target_link_libraries(wowlet PUBLIC ${X11_LIBRARIES})
endif()

if(APPLE)
    include(Deploy)
endif()

install(TARGETS wowlet
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        )


message(STATUS "=============================================")
message(STATUS "VERSION_MAJOR: ${VERSION_MAJOR}")
message(STATUS "VERSION_MINOR: ${VERSION_MINOR}")
message(STATUS "VERSION_REVISION: ${VERSION_REVISION}")
message(STATUS "STATIC: ${STATIC}")
message(STATUS "VERSION: ${VERSION}")
message(STATUS "Include Valve's OpenVR library: ${OPENVR}")
message(STATUS "This build is for Android: ${ANDROID}")
message(STATUS "This build is for testing the Android app on desktop: ${ANDROID_DEBUG}")
message(STATUS "TOR_BIN: ${TOR_BIN}")
message(STATUS "DONATE_BEG: ${DONATE_BEG}")
message(STATUS "=============================================")