// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <iostream>
#include <QResource>
#include <QApplication>
#include <QCoreApplication>
#include <QQmlComponent>
#include <QObject>
#include <QtCore>
#include <QtGui>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QFileInfo>
#include <QQuickView>
#include <QQuickItem>

#include "openvr.h"
#include "vr/openvr_init.h"
#include "vr/main.h"

#include "libwalletqt/TransactionInfo.h"
#include "libwalletqt/TransactionHistory.h"
#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "libwalletqt/WalletManager.h"

#include "utils/keysfiles.h"

namespace wowletvr {

    void check_error(int line, vr::EVRInitError error) { if (error != 0) printf("%d: error %s\n", line, VR_GetVRInitErrorAsSymbol(error)); }

    WowletVR::WowletVR(AppContext *ctx, QCommandLineParser *parser, QObject *parent) :
            QObject(parent), ctx(ctx), m_parser(parser) {
        AppContext::isQML = true;
        m_pClipboard = QGuiApplication::clipboard();
        desktopMode = m_parser->isSet("openvr-debug");

        // Init QML themes
        this->readThemes();

        // turn on auto tx commits
        ctx->autoCommitTx = true;

        // QR code scanning from screenshots
        m_qrScreenshotPreviewPath = ctx->configDirectoryVR + "/screenshot_preview";
        m_qrScreenshotImagePath = ctx->configDirectoryVR + "/screenshot";
        m_qrScreenshotTimer.setSingleShot(true);
        connect(&m_qrScreenshotTimer, &QTimer::timeout, this, &WowletVR::onCheckQRScreenshot);

        // write icon to disk so openvr overlay can use it
        auto icon = ":/assets/images/wowlet.png";
        if(Utils::fileExists(icon)) {
            QFile f(icon);
            QFileInfo fileInfo(f);
            auto icon_path = QDir(ctx->configDirectory).filePath(fileInfo.fileName());
            f.copy(icon_path);
            f.close();
        }

#ifdef Q_OS_WIN
        if(desktopMode)
            qputenv("QMLSCENE_DEVICE", "softwarecontext");  // virtualbox
#endif
        qDebug() << "QMLSCENE_DEVICE: " << qgetenv("QMLSCENE_DEVICE");
        qInfo() << "OPENSSL VERSION: " << QSslSocket::sslLibraryBuildVersionString();

        m_engine.rootContext()->setContextProperty("homePath", QDir::homePath());
        m_engine.rootContext()->setContextProperty("applicationDirectory", QApplication::applicationDirPath());
        m_engine.rootContext()->setContextProperty("idealThreadCount", QThread::idealThreadCount());
        m_engine.rootContext()->setContextProperty("qtRuntimeVersion", qVersion());
        m_engine.rootContext()->setContextProperty("ctx", ctx);

//        qmlRegisterType<clipboardAdapter>("moneroComponents.Clipboard", 1, 0, "Clipboard");
        m_engine.rootContext()->setContextProperty("WowletVR", this);
        qRegisterMetaType<NetworkType::Type>();
        qmlRegisterType<NetworkType>("wowlet.NetworkType", 1, 0, "NetworkType");

        qmlRegisterUncreatableType<WalletKeysFiles>("wowlet.WalletKeysFiles", 1, 0, "WalletKeysFiles", "WalletKeysFiles can't be instantiated directly");
        qmlRegisterUncreatableType<Wallet>("wowlet.Wallet", 1, 0, "Wallet", "Wallet can't be instantiated directly");
        qmlRegisterType<WalletManager>("wowlet.WalletManager", 1, 0, "WalletManager");

        qmlRegisterUncreatableType<TransactionHistoryProxyModel>("wowlet.TransactionHistoryProxyModel", 1, 0, "TransactionHistoryProxyModel", "TransactionHistoryProxyModel can't be instantiated directly");
        qmlRegisterUncreatableType<TransactionHistoryModel>("wowlet.TransactionHistoryModel", 1, 0, "TransactionHistoryModel", "TransactionHistoryModel can't be instantiated directly");
        qmlRegisterUncreatableType<TransactionInfo>("wowlet.TransactionInfo", 1, 0, "TransactionInfo", "TransactionHistory can't be instantiated directly");
        qmlRegisterUncreatableType<TransactionHistory>("wowlet.TransactionHistory", 1, 0, "TransactionHistory", "TransactionHistory can't be instantiated directly");

        qRegisterMetaType<PendingTransaction::Priority>();
        qRegisterMetaType<TransactionInfo::Direction>();
        qRegisterMetaType<TransactionHistoryModel::TransactionInfoRole>();

        if(!desktopMode) {
            if(!openvr_init::initializeOpenVR(openvr_init::OpenVrInitializationType::Overlay))
                throw std::runtime_error("Error: initializeOpenVR()");
        }

        m_controller = new wowletvr::OverlayController(desktopMode, m_engine);
        m_engine.rootContext()->setContextProperty("OverlayController", m_controller);

        auto widgetUrl = QUrl(QStringLiteral("qrc:///main"));
        m_component = new QQmlComponent(&m_engine, widgetUrl);

        this->errors = m_component->errors();
        for (auto &e : this->errors)
            qCritical() << "QML Error: " << e.toString().toStdString().c_str();
    }

    void WowletVR::render() {
        auto quickObj = m_component->create();
        QQuickItem *quickObjItem = qobject_cast<QQuickItem*>(quickObj);

        auto displayName = application_strings::applicationDisplayName;
        auto appKey = application_strings::applicationKey;

        if(desktopMode) {
            auto m_pWindow = new QQuickWindow();
            qobject_cast<QQuickItem *>(quickObj)->setParentItem(m_pWindow->contentItem());
            m_pWindow->setGeometry(0, 0,
                static_cast<int>(qobject_cast<QQuickItem *>(quickObj)->width()),
                static_cast<int>(qobject_cast<QQuickItem *>(quickObj)->height()));
            m_pWindow->show();
            return;
        }

        auto iconPath = ctx->configDirectory + "/wowlet.png";
        m_controller->SetWidget(quickObjItem, displayName, appKey, iconPath.toStdString());
    }

    void WowletVR::readThemes() {
        theme_names = QVariantList();
        themes = QVariantMap();

        auto lol = Utils::fileOpen(":/qml_themes.json");
        auto doc = QJsonDocument::fromJson(lol);
        QJsonObject obj = doc.object();

        foreach(const QString &themeName, obj.keys()) {
            theme_names << themeName;

            QJsonValue value = obj.value(themeName);
            QJsonObject theme = value.toObject();
            QVariantMap map;
            foreach(const QString &key, theme.keys()) {
                map[key] = theme.value(key).toString();
            }

            themes[themeName] = map;
        }
    }

    void WowletVR::takeQRScreenshot() {
        if(m_qrScreenshotTimer.isActive())
            return;

        m_controller->takeQRScreenshot(m_qrScreenshotPreviewPath, m_qrScreenshotImagePath);
        m_qrScreenshotTimer.start(1000);
    }

    void WowletVR::onCheckQRScreenshot() {
        qDebug() << "onCheckQRScreenshot()";
        QString msg;
        auto path = m_qrScreenshotPreviewPath + ".png";
        auto pathPreview = m_qrScreenshotPreviewPath + "_inverted.png";

        qDebug() << "path: " + path << " inverted: " + pathPreview;

        if(!Utils::fileExists(path)) {
            msg = "Screenshot was not saved to disk.";
            qWarning() << msg;
            emit qrScreenshotFailed(msg);
            return;
        }

        auto age = Utils::fileModifiedAge(path);
        if (age >= 5) {
            msg = "Screenshot on disk too old. Leftover from the last time?";
            qWarning() << msg;
            emit qrScreenshotFailed(msg);
            QFile file (path);
            file.remove();
            return;
        }

        auto results = m_qrDecoder.decodePNG(path);
        auto result = wowletvr::WowletVR::checkQRScreenshotResults(results);
        qDebug() << "QR code try #1: " << result;
        if(result.isEmpty()) {
            qDebug() << "trying to invert the image";
            QImage image(path);
            image.invertPixels();
            image.save(pathPreview);
            results = m_qrDecoder.decodePNG(pathPreview);
            result = wowletvr::WowletVR::checkQRScreenshotResults(results);
            qDebug() << "QR code try #2: " << result;
        }

        if(!result.isEmpty()) {
            qDebug() << "QR code decoded, trying address validation.";

            if(result.toLower().startsWith("wownero:"))
                result = result.remove(0, 8);

            if(WalletManager::addressValid(result, NetworkType::MAINNET)) {
                qDebug() << "QR code appears valid.";
                emit qrScreenshotSuccess(result);
                QFile file(path);
                file.remove();
                return;
            }
        }

        emit qrScreenshotFailed("No QR code could be detected.");
    }

    QString WowletVR::checkQRScreenshotResults(std::vector<std::string> results) {
        auto results_count = results.size();
        if(results_count == 1)
            return QString::fromStdString(results[0]);
        return "";
    }

    WowletVR::~WowletVR() {
        // bla
        int wegeg = 1;
    }
}
