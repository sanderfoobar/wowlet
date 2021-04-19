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

#include "libwalletqt/TransactionInfo.h"
#include "libwalletqt/TransactionHistory.h"
#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "libwalletqt/WalletManager.h"

#include "utils/keysfiles.h"
#include "mobile/main.h"

namespace mobile {

    Mobile::Mobile(AppContext *ctx, QCommandLineParser *parser, QObject *parent) :
            QObject(parent), ctx(ctx), m_parser(parser) {
        AppContext::isQML = true;
        m_pClipboard = QGuiApplication::clipboard();
        desktopMode = true;

        // turn on auto tx commits
        ctx->autoCommitTx = true;

        // QR code scanning from screenshots
        m_qrScreenshotPreviewPath = ctx->configDirectoryVR + "/screenshot_preview";
        m_qrScreenshotImagePath = ctx->configDirectoryVR + "/screenshot";
        m_qrScreenshotTimer.setSingleShot(true);

        qDebug() << "QMLSCENE_DEVICE: " << qgetenv("QMLSCENE_DEVICE");

        m_engine.rootContext()->setContextProperty("homePath", QDir::homePath());
        m_engine.rootContext()->setContextProperty("applicationDirectory", QApplication::applicationDirPath());
        m_engine.rootContext()->setContextProperty("idealThreadCount", QThread::idealThreadCount());
        m_engine.rootContext()->setContextProperty("qtRuntimeVersion", qVersion());
        m_engine.rootContext()->setContextProperty("ctx", ctx);

        m_engine.rootContext()->setContextProperty("Mobile", this);
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

        auto widgetUrl = QUrl(QStringLiteral("qrc:///main"));
        m_engine.load(widgetUrl);
        if (m_engine.rootObjects().isEmpty())
        {
            qCritical() << "Error: no root objects";
            return;
        }
        QObject *rootObject = m_engine.rootObjects().first();
        if (!rootObject)
        {
            qCritical() << "Error: no root objects";
            return;
        }


        int wege = 1;
    }

    void Mobile::takeQRScreenshot() {

    }

    void Mobile::onCheckQRScreenshot() {

    }

    QString Mobile::checkQRScreenshotResults(std::vector<std::string> results) {

    }

    Mobile::~Mobile() {
        // bla
        int wegeg = 1;
    }
}
