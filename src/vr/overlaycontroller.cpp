// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.
// Copyright (c) OpenVR Advanced Settings

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QQuickView>
#include <QApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QOpenGLExtraFunctions>
#include <QCursor>
#include <QProcess>
#include <QMessageBox>
#include <iostream>
#include <cmath>

#include "overlaycontroller.h"
#include <openvr.h>

// application namespace
namespace wowletvr
{

OverlayController::OverlayController(bool desktopMode, QQmlEngine& qmlEngine) :
        QObject(),
        m_desktopMode(desktopMode)
{
    // Arbitrarily chosen Max Length of Directory path, should be sufficient for
    // Any set-up
    const uint32_t maxLength = 16192;
    uint32_t requiredLength;

    char tempRuntimePath[maxLength];
    bool pathIsGood = vr::VR_GetRuntimePath( tempRuntimePath, maxLength, &requiredLength );

    // Throw Error If over 16k characters in path string
    if ( !pathIsGood )
    {
        qCritical() << "Error Finding VR Runtime Path, Attempting Recovery: ";
        uint32_t maxLengthRe = requiredLength;
        qInfo() << "Open VR reporting Required path length of: "
                    << maxLengthRe;
    }

    m_runtimePathUrl = QUrl::fromLocalFile( tempRuntimePath );
    qInfo() << "VR Runtime Path: " << m_runtimePathUrl.toLocalFile();

    QSurfaceFormat format;
    // Qt's QOpenGLPaintDevice is not compatible with OpenGL versions >= 3.0
    // NVIDIA does not care, but unfortunately AMD does
    // Are subtle changes to the semantics of OpenGL functions actually covered
    // by the compatibility profile, and this is an AMD bug?
    format.setVersion( 2, 1 );
    // format.setProfile( QSurfaceFormat::CompatibilityProfile );
    format.setDepthBufferSize( 16 );
    format.setStencilBufferSize( 8 );
    format.setSamples( 16 );

    m_openGLContext.setFormat( format );
    if ( !m_openGLContext.create() ) {
        throw std::runtime_error( "Could not create OpenGL context" );
    }

    // create an offscreen surface to attach the context and FBO to
    m_offscreenSurface.setFormat( m_openGLContext.format() );
    m_offscreenSurface.create();
    m_openGLContext.makeCurrent( &m_offscreenSurface );

    if (!vr::VROverlay()){
        QMessageBox::critical(nullptr, "Wowlet VR Overlay", "Is OpenVR running?");
        throw std::runtime_error( std::string( "No Overlay interface" ) );
    }

    // Set qml context
    qmlEngine.rootContext()->setContextProperty("applicationVersion", "1337");
    qmlEngine.rootContext()->setContextProperty("vrRuntimePath", getVRRuntimePathUrl());
}

OverlayController::~OverlayController() {
    Shutdown();
}

void OverlayController::exitApp() {
    Shutdown();
    QApplication::exit();

    qInfo() << "All systems exited.";
    exit( EXIT_SUCCESS );
    // Does not fallthrough
}

void OverlayController::Shutdown() {
    if (m_pRenderTimer)
    {
        disconnect( &m_renderControl,
                    SIGNAL( renderRequested() ),
                    this,
                    SLOT( OnRenderRequest() ) );
        disconnect( &m_renderControl,
                    SIGNAL( sceneChanged() ),
                    this,
                    SLOT( OnRenderRequest() ) );
        disconnect( m_pRenderTimer.get(),
                    SIGNAL( timeout() ),
                    this,
                    SLOT( renderOverlay() ) );
        m_pRenderTimer->stop();
        m_pRenderTimer.reset();
    }
    m_pFbo.reset();
}

void OverlayController::SetWidget(QQuickItem* quickItem, const std::string& name, const std::string& key, const std::string& iconPath)
{
    if ( !m_desktopMode )
    {
        vr::VROverlayError overlayError
            = vr::VROverlay()->CreateDashboardOverlay(
                key.c_str(),
                name.c_str(),
                &m_ulOverlayHandle,
                &m_ulOverlayThumbnailHandle );
        if ( overlayError != vr::VROverlayError_None )
        {
            if ( overlayError == vr::VROverlayError_KeyInUse )
            {
                QMessageBox::critical( nullptr,
                                       "Wowlet VR Overlay",
                                       "Another instance is already running." );
            }
            throw std::runtime_error( std::string(
                "Failed to create Overlay: "
                + std::string( vr::VROverlay()->GetOverlayErrorNameFromEnum(
                    overlayError ) ) ) );
        }
        vr::VROverlay()->SetOverlayWidthInMeters( m_ulOverlayHandle, 2.5f );
        vr::VROverlay()->SetOverlayInputMethod(
            m_ulOverlayHandle, vr::VROverlayInputMethod_Mouse );
        vr::VROverlay()->SetOverlayFlag(
            m_ulOverlayHandle,
            vr::VROverlayFlags_SendVRSmoothScrollEvents,
            true );

        // Overlay icon
        if (!iconPath.empty())
            vr::VROverlay()->SetOverlayFromFile( m_ulOverlayThumbnailHandle, iconPath.c_str() );

        // Too many render calls in too short time overwhelm Qt and an
        // assertion gets thrown. Therefore we use an timer to delay render
        // calls
        m_pRenderTimer.reset(new QTimer());
        m_pRenderTimer->setSingleShot( false );
        m_pRenderTimer->setInterval( 5 );
        connect( m_pRenderTimer.get(),
                 SIGNAL( timeout() ),
                 this,
                 SLOT( renderOverlay() ) );

        QOpenGLFramebufferObjectFormat fboFormat;
        fboFormat.setAttachment(
            QOpenGLFramebufferObject::CombinedDepthStencil );
        fboFormat.setTextureTarget( GL_TEXTURE_2D );
        m_pFbo.reset( new QOpenGLFramebufferObject(
            static_cast<int>( quickItem->width() ),
            static_cast<int>( quickItem->height() ),
            fboFormat ) );

        m_window.setRenderTarget( m_pFbo.get() );
        quickItem->setParentItem( m_window.contentItem() );
        m_window.setGeometry( 0,
                              0,
                              static_cast<int>( quickItem->width() ),
                              static_cast<int>( quickItem->height() ) );
        m_renderControl.initialize( &m_openGLContext );

        vr::HmdVector2_t vecWindowSize
            = { static_cast<float>( quickItem->width() ),
                static_cast<float>( quickItem->height() ) };
        vr::VROverlay()->SetOverlayMouseScale( m_ulOverlayHandle,
                                               &vecWindowSize );

        connect( &m_renderControl,
                 SIGNAL( renderRequested() ),
                 this,
                 SLOT( OnRenderRequest() ) );
        connect( &m_renderControl,
                 SIGNAL( sceneChanged() ),
                 this,
                 SLOT( OnRenderRequest() ) );

        m_pRenderTimer->start();
    }
}

void OverlayController::OnRenderRequest() {
    if ( m_pRenderTimer && !m_pRenderTimer->isActive() )
    {
        m_pRenderTimer->start();
    }
}

void OverlayController::renderOverlay() {
    if ( !m_desktopMode )
    {
        // skip rendering if the overlay isn't visible
        if ( !vr::VROverlay()
             || ( !vr::VROverlay()->IsOverlayVisible( m_ulOverlayHandle )
                  && !vr::VROverlay()->IsOverlayVisible(
                      m_ulOverlayThumbnailHandle ) ) )
            return;
        m_renderControl.polishItems();
        m_renderControl.sync();
        m_renderControl.render();

        GLuint unTexture = m_pFbo->texture();
        if ( unTexture != 0 )
        {
#if defined _WIN64 || defined _LP64
            // To avoid any compiler warning because of cast to a larger
            // pointer type (warning C4312 on VC)
            vr::Texture_t texture = { reinterpret_cast<void*>(
                                          static_cast<uint64_t>( unTexture ) ),
                                      vr::TextureType_OpenGL,
                                      vr::ColorSpace_Auto };
#else
            vr::Texture_t texture = { reinterpret_cast<void*>( unTexture ),
                                      vr::TextureType_OpenGL,
                                      vr::ColorSpace_Auto };
#endif
            vr::VROverlay()->SetOverlayTexture( m_ulOverlayHandle, &texture );
        }
        m_openGLContext.functions()->glFlush(); // We need to flush otherwise
                                                // the texture may be empty.*/

        if(m_customTickRateCounter % k_nonVsyncTickRate == 0) {
            mainEventLoop();
            m_customTickRateCounter = 0;
        } else {
            m_customTickRateCounter += 1;
        }
    }
}

bool OverlayController::pollNextEvent( vr::VROverlayHandle_t ulOverlayHandle,
                                       vr::VREvent_t* pEvent ) {
    if ( isDesktopMode() )
    {
        return vr::VRSystem()->PollNextEvent( pEvent, sizeof( vr::VREvent_t ) );
    }
    else
    {
        return vr::VROverlay()->PollNextOverlayEvent(
            ulOverlayHandle, pEvent, sizeof( vr::VREvent_t ) );
    }
}

QPoint OverlayController::getMousePositionForEvent( vr::VREvent_Mouse_t mouse ) {
    float y = mouse.y;
#ifdef __linux__
    float h = static_cast<float>( m_window.height() );
    y = h - y;
#endif
    return QPoint( static_cast<int>( mouse.x ), static_cast<int>( y ) );
}

void OverlayController::mainEventLoop() {
    if ( !vr::VRSystem() )
        return;

    vr::VREvent_t vrEvent;

    while ( pollNextEvent( m_ulOverlayHandle, &vrEvent ) ) {
        switch ( vrEvent.eventType )
        {
        case vr::VREvent_MouseMove:
        {
            QPoint ptNewMouse = getMousePositionForEvent( vrEvent.data.mouse );
            if ( ptNewMouse != m_ptLastMouse )
            {
                QMouseEvent mouseEvent( QEvent::MouseMove,
                                        ptNewMouse,
                                        m_window.mapToGlobal( ptNewMouse ),
                                        Qt::NoButton,
                                        m_lastMouseButtons,
                                        nullptr );
                m_ptLastMouse = ptNewMouse;
                QCoreApplication::sendEvent( &m_window, &mouseEvent );
                OnRenderRequest();
            }
        }
        break;

        case vr::VREvent_MouseButtonDown:
        {
            QPoint ptNewMouse = getMousePositionForEvent( vrEvent.data.mouse );
            Qt::MouseButton button
                = vrEvent.data.mouse.button == vr::VRMouseButton_Right
                      ? Qt::RightButton
                      : Qt::LeftButton;
            m_lastMouseButtons |= button;
            QMouseEvent mouseEvent( QEvent::MouseButtonPress,
                                    ptNewMouse,
                                    m_window.mapToGlobal( ptNewMouse ),
                                    button,
                                    m_lastMouseButtons,
                                    nullptr );
            QCoreApplication::sendEvent( &m_window, &mouseEvent );
        }
        break;

        case vr::VREvent_MouseButtonUp:
        {
            QPoint ptNewMouse = getMousePositionForEvent( vrEvent.data.mouse );
            Qt::MouseButton button
                = vrEvent.data.mouse.button == vr::VRMouseButton_Right
                      ? Qt::RightButton
                      : Qt::LeftButton;
            m_lastMouseButtons &= ~button;
            QMouseEvent mouseEvent( QEvent::MouseButtonRelease,
                                    ptNewMouse,
                                    m_window.mapToGlobal( ptNewMouse ),
                                    button,
                                    m_lastMouseButtons,
                                    nullptr );
            QCoreApplication::sendEvent( &m_window, &mouseEvent );
        }
        break;

        case vr::VREvent_ScrollSmooth:
        {
            // Wheel speed is defined as 1/8 of a degree
            QWheelEvent wheelEvent(
                m_ptLastMouse,
                m_window.mapToGlobal( m_ptLastMouse ),
                QPoint(),
                QPoint( static_cast<int>( vrEvent.data.scroll.xdelta
                                          * ( 360.0f * 8.0f ) ),
                        static_cast<int>( vrEvent.data.scroll.ydelta
                                          * ( 360.0f * 8.0f ) ) ),
                0,
                Qt::Vertical,
                m_lastMouseButtons,
                nullptr );
            QCoreApplication::sendEvent( &m_window, &wheelEvent );
        }
        break;

        case vr::VREvent_OverlayShown:
        {
            m_window.update();
        }
        break;

        case vr::VREvent_Quit:
        {
            qInfo() << "Received quit request.";
            vr::VRSystem()->AcknowledgeQuit_Exiting(); // Let us buy some
                                                       // time just in case

            exitApp();
            // Won't fallthrough, but also exitApp() wont, but QT won't
            // acknowledge
            exit( EXIT_SUCCESS );
        }

        case vr::VREvent_DashboardActivated:
        {
            qDebug() << "Dashboard activated";
            emit dashboardActivated();
            m_dashboardVisible = true;
        }
        break;

        case vr::VREvent_DashboardDeactivated:
        {
            qDebug() << "Dashboard deactivated";
            emit dashboardDeactivated();
            m_dashboardVisible = false;
        }
        break;

        case vr::VREvent_KeyboardDone:
        {
            qDebug() << "VREvent_KeyboardDone";
            char keyboardBuffer[1024];
            vr::VROverlay()->GetKeyboardText( keyboardBuffer, 1024 );
            qDebug() << "emit keyBoardInputSignal()";
            emit keyBoardInputSignal( QString( keyboardBuffer ),
                                      static_cast<unsigned long>(
                                          vrEvent.data.keyboard.uUserValue ) );
        }
        break;
        }
    }

    if ( m_ulOverlayThumbnailHandle != vr::k_ulOverlayHandleInvalid ) {
        while ( vr::VROverlay()->PollNextOverlayEvent(
            m_ulOverlayThumbnailHandle, &vrEvent, sizeof( vrEvent ) ) )
        {
            switch ( vrEvent.eventType )
            {
            case vr::VREvent_OverlayShown:
            {
                m_window.update();
            }
            break;
            }
        }
    }
}

void OverlayController::showKeyboard(QString existingText, unsigned long userValue)
{
    vr::VROverlay()->ShowKeyboardForOverlay(
            m_ulOverlayHandle,
            vr::k_EGamepadTextInputModeNormal,
            vr::k_EGamepadTextInputLineModeSingleLine,
            0,
            "Wowlet VR",
            1024,
            existingText.toStdString().c_str(),
            userValue);
    setKeyboardPos();
}

void OverlayController::setKeyboardPos()
{
    vr::HmdVector2_t emptyvec;
    emptyvec.v[0] = 0;
    emptyvec.v[1] = 0;
    vr::HmdRect2_t empty;
    empty.vTopLeft = emptyvec;
    empty.vBottomRight = emptyvec;
    vr::VROverlay()->SetKeyboardPositionForOverlay( m_ulOverlayHandle, empty );
}

QUrl OverlayController::getVRRuntimePathUrl() {
    return m_runtimePathUrl;
}

bool OverlayController::soundDisabled() {
    return true;
}

const vr::VROverlayHandle_t& OverlayController::overlayHandle() {
    return m_ulOverlayHandle;
}

const vr::VROverlayHandle_t& OverlayController::overlayThumbnailHandle() {
    return m_ulOverlayThumbnailHandle;
}

} // namespace wowletvr
