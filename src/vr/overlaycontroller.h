// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.
// Copyright (c) OpenVR Advanced Settings

#pragma once

#include <openvr.h>
#include <QtCore/QtCore>
// because of incompatibilities with QtOpenGL and GLEW we need to cherry pick includes
#include <QVector2D>
#include <QMatrix4x4>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QObject>
#include <QQmlEngine>
#include <QtGui/QOpenGLContext>
#include <QtWidgets/QGraphicsScene>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <memory>

#include "openvr_init.h"
#include "vr/utils/paths.h"

namespace application_strings
{
constexpr auto applicationOrganizationName = "Wownero";
constexpr auto applicationName = "Wowlet VR";
constexpr const char* applicationKey = "steam.overlay.1001337";
constexpr const char* applicationDisplayName = "Wowlet VR";

constexpr const char* applicationVersionString = "1337";

} // namespace application_strings

constexpr int k_nonVsyncTickRate = 20;

// application namespace
namespace wowletvr
{

class OverlayController : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool m_desktopMode READ isDesktopMode )

public:
    OverlayController(bool desktopMode, QQmlEngine& qmlEngine);
    virtual ~OverlayController();

    void Shutdown();

    Q_INVOKABLE void exitApp();
    void takeQRScreenshot(const QString &previewPath, const QString &imagePath);

    bool isDashboardVisible()
    {
        return m_dashboardVisible;
    }

    void SetWidget(QQuickItem* quickItem, const std::string& name, const std::string& key = "", const std::string& iconPath = "");

    bool isDesktopMode()
    {
        return m_desktopMode;
    }

    Q_INVOKABLE QUrl getVRRuntimePathUrl();
    Q_INVOKABLE bool soundDisabled();

    const vr::VROverlayHandle_t& overlayHandle();
    const vr::VROverlayHandle_t& overlayThumbnailHandle();

    bool pollNextEvent(vr::VROverlayHandle_t ulOverlayHandle, vr::VREvent_t* pEvent );
    void mainEventLoop();

private:
    vr::VROverlayHandle_t m_ulOverlayHandle = vr::k_ulOverlayHandleInvalid;
    vr::VROverlayHandle_t m_ulOverlayThumbnailHandle
        = vr::k_ulOverlayHandleInvalid;

    QQuickRenderControl m_renderControl;
    QQuickWindow m_window{ &m_renderControl };
    std::unique_ptr<QOpenGLFramebufferObject> m_pFbo;
    QOpenGLContext m_openGLContext;
    QOffscreenSurface m_offscreenSurface;

    std::unique_ptr<QTimer> m_pRenderTimer;
    bool m_dashboardVisible = false;

    QPoint m_ptLastMouse;
    Qt::MouseButtons m_lastMouseButtons = nullptr;

    bool m_desktopMode;

    QUrl m_runtimePathUrl;

    uint64_t m_customTickRateCounter = 0;
    uint64_t m_currentFrame = 0;
    uint64_t m_lastFrame = 0;

    QNetworkAccessManager* netManager = new QNetworkAccessManager( this );
    QJsonDocument m_remoteVersionJsonDocument = QJsonDocument();
    QJsonObject m_remoteVersionJsonObject;

private:
    QPoint getMousePositionForEvent( vr::VREvent_Mouse_t mouse );

    bool m_exclusiveState = false;
    bool m_keyPressOneState = false;
    bool m_keyPressTwoState = false;

public slots:
    void renderOverlay();
    void OnRenderRequest();

    void showKeyboard( QString existingText, unsigned long userValue = 0 );
    void setKeyboardPos();

signals:
    void keyBoardInputSignal( QString input, unsigned long userValue = 0 );
    void dashboardDeactivated();
    void dashboardActivated();
};

} // namespace wowletvr
