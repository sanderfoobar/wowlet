// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QResource>
#include <QApplication>
#include <QtCore>
#include <QtGui>

#include "config-wowlet.h"
#include "mainwindow.h"
#include "cli.h"

#ifdef HAS_OPENVR
#include "vr/main.h"
#endif

#ifdef HAS_ANDROID_DEBUG
#include "mobile/main.h"
#elif HAS_ANDROID
#include "mobile/main.h"
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#if defined(Q_OS_LINUX) && defined(STATIC)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(assets);
    qputenv("QML_DISABLE_DISK_CACHE", "1");

#if defined(Q_OS_MAC) && defined(HAS_TOR_BIN)
    Q_INIT_RESOURCE(assets_tor_macos);
#elif defined(HAS_TOR_BIN)
    Q_INIT_RESOURCE(assets_tor);
#endif

#ifdef _WIN32
if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}
#endif

    QStringList argv_;
    for(int i = 0; i != argc; i++){
        argv_ << QString::fromStdString(argv[i]);
    }

    QCoreApplication::setApplicationName("wowlet");
    QCoreApplication::setOrganizationDomain("wownero.org");
    QCoreApplication::setOrganizationName("wownero.org");

    QCommandLineParser parser;
    parser.setApplicationDescription("wowlet");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption useLocalTorOption(QStringList() << "use-local-tor", "Use system wide installed Tor instead of the bundled.");
    parser.addOption(useLocalTorOption);

    QCommandLineOption torPortOption(QStringList() << "tor-port", "Port of running Tor instance.", "torPort");
    parser.addOption(torPortOption);

    QCommandLineOption debugModeOption(QStringList() << "debug", "Run program in debug mode.");
    parser.addOption(debugModeOption);

    QCommandLineOption quietModeOption(QStringList() << "quiet", "Limit console output");
    parser.addOption(quietModeOption);

//    QCommandLineOption stagenetOption(QStringList() << "stagenet", "Stagenet is for development purposes only.");
//    parser.addOption(stagenetOption);
//
//    QCommandLineOption testnetOption(QStringList() << "testnet", "Testnet is for development purposes only.");
//    parser.addOption(testnetOption);

    QCommandLineOption walletPathOption(QStringList() << "wallet-file", "Path to wallet keys file.", "file");
    parser.addOption(walletPathOption);

    QCommandLineOption walletPasswordOption(QStringList() << "password", "Wallet password (escape/quote as needed)", "password");
    parser.addOption(walletPasswordOption);

    QCommandLineOption daemonAddressOption(QStringList() << "daemon-address", "Daemon address (IPv4:port)", "daemonAddress");
    parser.addOption(daemonAddressOption);

    QCommandLineOption exportContactsOption(QStringList() << "export-contacts", "Output wallet contacts as CSV to specified path.", "file");
    parser.addOption(exportContactsOption);

    QCommandLineOption exportTxHistoryOption(QStringList() << "export-txhistory", "Output wallet transaction history as CSV to specified path.", "file");
    parser.addOption(exportTxHistoryOption);

    QCommandLineOption backgroundOption(QStringList() << "daemon", "Start WOWlet in the background and start a websocket server (IPv4:port)", "backgroundAddress");
    parser.addOption(backgroundOption);

    QCommandLineOption backgroundPasswordOption(QStringList() << "daemon-password", "Password for connecting to the wowlet websocket service", "backgroundPassword");
    parser.addOption(backgroundPasswordOption);

    QCommandLineOption openVROption(QStringList() << "openvr", "Start Wowlet OpenVR");
    parser.addOption(openVROption);

    QCommandLineOption openVRDebugOption(QStringList() << "openvr-debug", "Start the Wowlet VR interface without initializing OpenVR - for debugging purposes. Requires -DOPENVR=ON CMake definition.");
    parser.addOption(openVRDebugOption);

    QCommandLineOption androidDebugOption(QStringList() << "android-debug", "Start the Android interface without actually running on Android - for debugging purposes. Requires -DANDROID_DEBUG=ON CMake definition.");
    parser.addOption(androidDebugOption);

    QCommandLineOption backendHostOption(QStringList() << "backend-host", "specify your own `wowlet-backend` host", "backend-host");
    parser.addOption(backendHostOption);

    QCommandLineOption backendPortOption(QStringList() << "backend-port", "specify your own `wowlet-backend` port", "backend-port");
    parser.addOption(backendPortOption);

    QCommandLineOption backendTLS(QStringList() << "backend-tls", "`wowlet-backend` is running via TLS? 'wss://' and 'https://' will be used.", "backend-tls");
    parser.addOption(backendTLS);

    auto parsed = parser.parse(argv_);
    if(!parsed) {
        qCritical() << parser.errorText();
        exit(1);
    }

    const QStringList args = parser.positionalArguments();
    bool debugMode = parser.isSet(debugModeOption);
    bool localTor = parser.isSet(useLocalTorOption);
    bool stagenet = false;
    bool testnet = false;
    bool quiet = parser.isSet(quietModeOption);
    bool exportContacts = parser.isSet(exportContactsOption);
    bool exportTxHistory = parser.isSet(exportTxHistoryOption);
    bool backgroundAddressEnabled = parser.isSet(backgroundOption);
    bool openVREnabled = parser.isSet(openVROption);
    bool cliMode = exportContacts || exportTxHistory || backgroundAddressEnabled;
    bool androidDebug = parser.isSet(androidDebugOption);
    bool android = false;
#ifdef __ANDROID__
    android = true;
#endif
    qRegisterMetaType<QVector<QString>>();

#ifdef __ANDROID__
    if(android || androidDebug) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QGuiApplication mobile_app(argc, argv);
        auto *ctx = new AppContext(&parser);
        auto *mobile = new mobile::Mobile(ctx, &parser, &mobile_app);
        return mobile_app.exec();
    }
#endif

    if(openVREnabled) {
#ifdef HAS_OPENVR
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QApplication vr_app(argc, argv);
        auto *ctx = new AppContext(&parser);
        auto *vr = new wowletvr::WowletVR(ctx, &parser, &vr_app);
        if(vr->errors.length() > 0)
            return 1;

        vr->render();
        return vr_app.exec();
#else
        qCritical() << "Wowlet compiled without OpenVR support.";
        exit(1);
#endif
    }

    if(cliMode) {
        auto *ctx = new AppContext(&parser);
        QCoreApplication cli_app(argc, argv);

        ctx->applicationPath = QString(argv[0]);
        ctx->isDebug = debugMode;

        auto *cli = new CLI(ctx, &cli_app);
        QObject::connect(cli, &CLI::closeApplication, &cli_app, &QCoreApplication::quit);

        if(exportContacts) {
            if(!quiet)
                qInfo() << "CLI mode: Address book export";
            cli->mode = CLIMode::CLIModeExportContacts;
            QTimer::singleShot(0, cli, &CLI::run);
        } else if(exportTxHistory) {
            if(!quiet)
                qInfo() << "CLI mode: Transaction history export";
            cli->mode = CLIMode::CLIModeExportTxHistory;
            QTimer::singleShot(0, cli, &CLI::run);
        } else if(backgroundAddressEnabled) {
            if(!quiet)
                qInfo() << "CLI mode: daemonize";
            cli->mode = CLIMode::CLIDaemonize;

            auto backgroundHostPort = parser.value(backgroundOption);
            if(!backgroundHostPort.contains(":")) {
                qCritical() << "the format is: --background ipv4:port";
                return 1;
            }

            auto spl = backgroundHostPort.split(":");
            cli->backgroundWebsocketAddress = spl.at(0);
            cli->backgroundWebsocketPort = (quint16) spl.at(1).toInt();
            cli->backgroundWebsocketPassword = parser.value(backgroundPasswordOption);
            if(cli->backgroundWebsocketPassword.isEmpty()) {
                qCritical() << "--daemon-password needs to be set when using --daemon";
                return 1;
            }

            QTimer::singleShot(0, cli, &CLI::run);
        }

        return QCoreApplication::exec();
    }

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
    QApplication::setDesktopSettingsAware(true); // use system font
    QApplication::setApplicationVersion(WOWLET_VERSION);

    QApplication app(argc, argv);

    parser.process(app); // Parse again for --help and --version

    if(!quiet) {
        QMap<QString, QString> info;
        info["Qt"] = QT_VERSION_STR;
        info["WOWlet"] = WOWLET_VERSION;
        if (stagenet) info["Mode"] = "Stagenet";
        else if (testnet) info["Mode"] = "Testnet";
        else info["Mode"] = "Mainnet";
#ifndef QT_NO_SSL
        info["SSL"] = QSslSocket::sslLibraryVersionString();
        info["SSL build"] = QSslSocket::sslLibraryBuildVersionString();
#endif
        for (const auto &k: info.keys())
            qWarning().nospace().noquote() << QString("%1: %2").arg(k).arg(info[k]);
    }

    auto *ctx = new AppContext(&parser);
    ctx->applicationPath = QString(argv[0]);
    ctx->isDebug = debugMode;

#if defined(Q_OS_MAC)
    // For some odd reason, if we don't do this, QPushButton's
    // need to be clicked *twice* in order to fire ?!
    QFont fontDef = QApplication::font();
    fontDef.setPointSize(fontDef.pointSize() + 1);
    QApplication::setFont(fontDef);
#endif

    qInstallMessageHandler(Utils::applicationLogHandler);

    auto *mainWindow = new MainWindow(ctx);
    return QApplication::exec();
}
