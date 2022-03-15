// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QDir>
#include <QMessageBox>

#include "appcontext.h"
#include "globals.h"
#include "config-wowlet.h"

// libwalletqt
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/Subaddress.h"
#include "libwalletqt/Coins.h"
#include "model/TransactionHistoryModel.h"
#include "model/SubaddressModel.h"


Prices *AppContext::prices = nullptr;
WalletKeysFilesModel *AppContext::wallets = nullptr;
TxFiatHistory *AppContext::txFiatHistory = nullptr;
double AppContext::balance = 0;
QMap<QString, QString> AppContext::txDescriptionCache;
QMap<QString, QString> AppContext::txCache;
bool AppContext::isQML = false;

AppContext::AppContext(QCommandLineParser *cmdargs) {
    this->m_walletKeysFilesModel = new WalletKeysFilesModel(this, this);
    this->network = new QNetworkAccessManager();
    this->networkClearnet = new QNetworkAccessManager();
    this->cmdargs = cmdargs;
    AppContext::isQML = false;

    // OS & env
#if defined(Q_OS_MAC)
    this->isMac = true;
    this->isTorSocks = qgetenv("DYLD_INSERT_LIBRARIES").indexOf("libtorsocks") >= 0;
#elif __ANDROID__
    this->isAndroid = true;
#elif defined(Q_OS_LINUX)
    this->isLinux = true;
    this->isTorSocks = qgetenv("LD_PRELOAD").indexOf("libtorsocks") >= 0;
    this->isTails = TailsOS::detect();
    this->isWhonix = WhonixOS::detect();
#elif defined(Q_OS_WIN)
    this->isWindows = true;
    this->isTorSocks = false;
#endif
    this->androidDebug = cmdargs->isSet("android-debug");

    // Paths
    this->pathGenericData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    this->configRoot = QDir::homePath();
    this->accountName = Utils::getUnixAccountName();
    this->homeDir = QDir::homePath();

    this->configDirectory = QString("%1/.config/wowlet/").arg(this->configRoot);
    this->configDirectoryVR = QString("%1%2").arg(this->configDirectory, "vr");

    if (isTails) this->setupPathsTails();

    QString walletDir = config()->get(Config::walletDirectory).toString();
    if(walletDir.isEmpty()) {
        if (isAndroid && !androidDebug) setupPathsAndroid();
        else if (isWindows) setupPathsWindows();
        else if (isLinux || isMac) setupPathsUnix();
    } else {
        this->defaultWalletDir = walletDir;
        this->defaultWalletDirRoot = walletDir;
    }

#ifdef __ANDROID__
    // can haz disk I/O?
    QVector<QString> perms = {
        "android.permission.WRITE_EXTERNAL_STORAGE",
        "android.permission.READ_EXTERNAL_STORAGE"
    };
    Utils::androidAskPermissions(perms);
#endif

    // Create wallet dirs
    qDebug() << "creating " << defaultWalletDir;
    if (!QDir().mkpath(defaultWalletDir))
        qCritical() << "Unable to create dir: " << defaultWalletDir;

    // Create some directories
    createConfigDirectory(this->configDirectory);
    this->networkType = NetworkType::MAINNET;

    qDebug() << "configRoot: " << this->configRoot;
    qDebug() << "homeDir: " << this->homeDir;
    qDebug() << "customWalletDir: " << walletDir;
    qDebug() << "defaultWalletDir: " << this->defaultWalletDir;
    qDebug() << "defaultWalletDirRoot: " << this->defaultWalletDirRoot;
    qDebug() << "configDirectory: " << this->configDirectory;

//    auto nodeSourceUInt = config()->get(Config::nodeSource).toUInt();
//    AppContext::nodeSource = static_cast<NodeSource>(nodeSourceUInt);
    this->nodes = new Nodes(this, this->networkClearnet);
    connect(this, &AppContext::nodeSourceChanged, this->nodes, &Nodes::onNodeSourceChanged);
    connect(this, &AppContext::setCustomNodes, this->nodes, &Nodes::setCustomNodes);

    // init backend URLs
    if(cmdargs->isSet("backend-host"))
      this->backendHost = cmdargs->value("backend-host");
    if(cmdargs->isSet("backend-host"))
      this->backendPort = cmdargs->value("backend-port").toUInt();
    if(cmdargs->isSet("backend-tls"))
      this->backendTLS = true;

    backendWSUrl = this->backendTLS ? "wss://" : "ws://";
    backendWSUrl += QString("%1:%2").arg(this->backendHost).arg(this->backendPort);
    backendHTTPUrl = this->backendTLS ? "https://" : "http://";
    backendHTTPUrl += QString("%1:%2").arg(this->backendHost).arg(this->backendPort);

    // init websocket client
    this->ws = new WSClient(this);
    connect(this->ws, &WSClient::WSMessage, this, &AppContext::onWSMessage);
    connect(this->ws, &WSClient::connectionEstablished, this, &AppContext::wsConnected);
    connect(this->ws, &WSClient::closed, this, &AppContext::wsDisconnected);

    // Store the wallet every 2 minutes
    m_storeTimer.start(2 * 60 * 1000);
    connect(&m_storeTimer, &QTimer::timeout, [this](){
        this->storeWallet();
    });

    // If system clock skewed for >= 300 seconds, assume a wake-up from hibernate and reload the websocket connection
    if(!this->isAndroid)
        m_hibernateTimer.start(3 * 1000);

    m_hibernatePreviousTime = std::chrono::steady_clock::now();
    connect(&m_hibernateTimer, &QTimer::timeout, [this]() {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = now - m_hibernatePreviousTime;

        if(elapsed >= m_hibernateDetectInterval) {
            qCritical() << "Clock skew detected, resetting websocket connection";
            this->ws->webSocket.abort();
            this->ws->start();
        }

        m_hibernatePreviousTime = now;
    });

    // restore height lookup
    this->initRestoreHeights();

    // price history lookup
    auto genesis_timestamp = this->restoreHeights[NetworkType::Type::MAINNET]->data.firstKey();
    AppContext::txFiatHistory = new TxFiatHistory(genesis_timestamp, this->configDirectory);
    connect(this->ws, &WSClient::connectionEstablished, AppContext::txFiatHistory, &TxFiatHistory::onUpdateDatabase);
    connect(AppContext::txFiatHistory, &TxFiatHistory::requestYear, [=](int year){
        QByteArray data = QString(R"({"cmd": "txFiatHistory", "data": {"year": %1}})").arg(year).toUtf8();
        this->ws->sendMsg(data);
    });
    connect(AppContext::txFiatHistory, &TxFiatHistory::requestYearMonth, [=](int year, int month) {
        QByteArray data = QString(R"({"cmd": "txFiatHistory", "data": {"year": %1, "month": %2}})").arg(year).arg(month).toUtf8();
        this->ws->sendMsg(data);
    });

    // fiat/crypto lookup
    AppContext::prices = new Prices();

    // XMRig
    this->XMRig = new XmRig(this->configDirectory, this);
    this->XMRig->prepare();

    this->walletManager = WalletManager::instance();
    QString logPath = QString("%1/daemon.log").arg(configDirectory);
    Monero::Utils::onStartup();
    Monero::Wallet::init("", "wowlet", logPath.toStdString(), true);

    bool logLevelFromEnv;
    int logLevel = qEnvironmentVariableIntValue("MONERO_LOG_LEVEL", &logLevelFromEnv);
    if(this->cmdargs->isSet("quiet"))
        this->walletManager->setLogLevel(-1);
    else if (logLevelFromEnv && logLevel >= 0 && logLevel <= Monero::WalletManagerFactory::LogLevel_Max)
        Monero::WalletManagerFactory::setLogLevel(logLevel);

    connect(this, &AppContext::createTransactionError, this, &AppContext::onCreateTransactionError);

    // libwallet connects
    connect(this->walletManager, &WalletManager::walletOpened, this, &AppContext::onWalletOpened);

    // hideOnClose
    auto hideOnClose = config()->get(Config::hideOnClose).toBool();
    if(hideOnClose)
        QApplication::setQuitOnLastWindowClosed(false);
}

void AppContext::initTor() {
    this->tor = new Tor(this, this);
    this->tor->start();

    if (!isWhonix && backendHost.contains(".onion")) {
        qDebug() << "'backend-host' did not contain '.onion' - running without Tor proxy.";
        this->networkProxy = new QNetworkProxy(QNetworkProxy::Socks5Proxy, Tor::torHost, Tor::torPort);
        this->network->setProxy(*networkProxy);
        this->ws->webSocket.setProxy(*networkProxy);
    }
}

void AppContext::initWS() {
    this->ws->start();
}

void AppContext::onCancelTransaction(PendingTransaction *tx, const QVector<QString> &address) {
    // tx cancelled by user
    double amount = tx->amount() / globals::cdiv;
    emit createTransactionCancelled(address, amount);
    this->currentWallet->disposeTransaction(tx);
}

void AppContext::onSweepOutput(const QString &keyImage, QString address, bool churn, int outputs) const {
    if(this->currentWallet == nullptr){
        qCritical() << "Cannot create transaction; no wallet loaded";
        return;
    }

    if (churn) {
        address = this->currentWallet->address(0, 0); // primary address
    }

    qCritical() << "Creating transaction";
    this->currentWallet->createTransactionSingleAsync(keyImage, address, outputs, this->tx_priority);
}

void AppContext::onCreateTransaction(QString address, quint64 amount, QString description, bool all) {
    // tx creation
    this->tmpTxDescription = description;

    if(this->currentWallet == nullptr) {
        emit createTransactionError("Cannot create transaction; no wallet loaded");
        return;
    }

    if (!all && amount == 0) {
        emit createTransactionError("Cannot send nothing");
        return;
    }

    auto balance = this->currentWallet->balance();
    auto unlocked_balance = this->currentWallet->unlockedBalance();
    if(!all && amount > unlocked_balance) {
        emit createTransactionError("Not enough money to spend");
        return;
    } else if(unlocked_balance == 0) {
        emit createTransactionError("No money to spend");
        return;
    }

    qDebug() << "creating tx";
    if (all)
        this->currentWallet->createTransactionAllAsync(address, "", this->tx_mixin, this->tx_priority);
    else
        this->currentWallet->createTransactionAsync(address, "", amount, this->tx_mixin, this->tx_priority);

    emit initiateTransaction();
}

void AppContext::onCreateTransactionMultiDest(const QVector<QString> &addresses, const QVector<quint64> &amounts, const QString &description) {
    this->tmpTxDescription = description;

    if (this->currentWallet == nullptr) {
        emit createTransactionError("Cannot create transaction; no wallet loaded");
        return;
    }

    quint64 total_amount = 0;
    for (auto &amount : amounts) {
        total_amount += amount;
    }

    auto unlocked_balance = this->currentWallet->unlockedBalance();
    if (total_amount > unlocked_balance) {
        emit createTransactionError("Not enough money to spend");
    }

    qDebug() << "Creating tx";
    this->currentWallet->createTransactionMultiDestAsync(addresses, amounts, this->tx_priority);

    emit initiateTransaction();
}

void AppContext::onCreateTransactionError(const QString &msg) {
    this->tmpTxDescription = "";
    emit endTransaction();
}

void AppContext::closeWallet(bool emitClosedSignal, bool storeWallet) {
    if (this->currentWallet == nullptr)
        return;

    emit walletAboutToClose();

    if (storeWallet) {
        this->storeWallet();
    }

    this->currentWallet->disconnect();
    this->walletManager->closeWallet();
    this->currentWallet = nullptr;

    if (emitClosedSignal)
        emit walletClosed();
}

void AppContext::onOpenWallet(const QString &path, const QString &password){
    if(this->currentWallet != nullptr){
        emit walletOpenedError("There is an active wallet opened.");
        return;
    }

    if(!Utils::fileExists(path)) {
        emit walletOpenedError(QString("Wallet not found: %1").arg(path));
        return;
    }

    if (password.isEmpty()) {
        this->walletPassword = "";
    }

    config()->set(Config::firstRun, false);

    this->walletPath = path;
    this->walletManager->openWalletAsync(path, password, this->networkType, 1);
}

void AppContext::onPreferredFiatCurrencyChanged(const QString &symbol) {
    if(this->currentWallet) {
        auto *model = this->currentWallet->transactionHistoryModel();
        if(model != nullptr) {
            model->preferredFiatSymbol = symbol;
        }
    }
}

void AppContext::onWalletOpened(Wallet *wallet) {
    auto state = wallet->status();
    if (state != Wallet::Status_Ok) {
        auto errMsg = wallet->errorString();
        if(errMsg == QString("basic_string::_M_replace_aux") || errMsg == QString("std::bad_alloc")) {
            qCritical() << errMsg;
            this->walletManager->clearWalletCache(this->walletPath);
            errMsg = QString("%1\n\nAttempted to clean wallet cache. Please restart WOWlet.").arg(errMsg);
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else if(errMsg.contains("wallet cannot be opened as")) {
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else if(errMsg.contains("is opened by another wallet program")) {
            this->closeWallet(false);
            emit walletOpenedError(errMsg);
        } else {
            this->closeWallet(false);
            emit walletOpenPasswordNeeded(!this->walletPassword.isEmpty(), wallet->path());
        }
        return;
    }

    this->refreshed = false;
    this->currentWallet = wallet;
    this->walletPath = this->currentWallet->path() + ".keys";
    QFileInfo fileInfo(this->currentWallet->path());
    this->walletName = fileInfo.fileName();
    this->walletViewOnly = this->currentWallet->viewOnly();
    config()->set(Config::walletPath, this->walletPath);

    connect(this->currentWallet, &Wallet::moneySpent, this, &AppContext::onMoneySpent);
    connect(this->currentWallet, &Wallet::moneyReceived, this, &AppContext::onMoneyReceived);
    connect(this->currentWallet, &Wallet::unconfirmedMoneyReceived, this, &AppContext::onUnconfirmedMoneyReceived);
    connect(this->currentWallet, &Wallet::newBlock, this, &AppContext::onWalletNewBlock);
    connect(this->currentWallet, &Wallet::updated, this, &AppContext::onWalletUpdate);
    connect(this->currentWallet, &Wallet::refreshed, this, &AppContext::onWalletRefreshed);
    connect(this->currentWallet, &Wallet::transactionCommitted, this, &AppContext::onTransactionCommitted);
    connect(this->currentWallet, &Wallet::heightRefreshed, this, &AppContext::onHeightRefreshed);
    connect(this->currentWallet, &Wallet::transactionCreated, this, &AppContext::onTransactionCreated);

    emit walletOpened(wallet);

    connect(this->currentWallet, &Wallet::connectionStatusChanged, [this]{
        this->nodes->autoConnect();
    });
    this->nodes->connectToNode();
    this->updateBalance();

#ifdef DONATE_BEG
    this->donateBeg();
#endif

    // force trigger preferredFiat signal for history model
    this->onPreferredFiatCurrencyChanged(config()->get(Config::preferredFiatCurrency).toString());
    this->setWindowTitle();
}

void AppContext::setWindowTitle(bool mining) {
    QFileInfo fileInfo(this->walletPath);
    auto title = QString("WOWlet - [%1]").arg(fileInfo.fileName());
    if(this->walletViewOnly)
        title += " [view-only]";
    if(mining)
        title += " [mining]";

    emit setTitle(title);
}

void AppContext::onWSMessage(const QJsonObject &msg) {
    QString cmd = msg.value("cmd").toString();

    if(cmd == "blockheights") {
        auto heights = msg.value("data").toObject();
        auto mainnet = heights.value("mainnet").toInt();
        auto stagenet = heights.value("stagenet").toInt();
        auto changed = false;

        if(!this->heights.contains("mainnet")) {
            this->heights["mainnet"] = mainnet;
            changed = true;
        }
        else {
            if (mainnet > this->heights["mainnet"]) {
                this->heights["mainnet"] = mainnet;
                changed = true;
            }
        }
        if(!this->heights.contains("stagenet")) {
            this->heights["stagenet"] = stagenet;
            changed = true;
        }
        else {
            if (stagenet > this->heights["stagenet"]) {
                this->heights["stagenet"] = stagenet;
                changed = true;
            }
        }

        if(changed)
            emit blockHeightWSUpdated(this->heights);
    }
    else if(cmd == "yellwow") {
        this->yellowPagesData = msg.value("data").toArray();
        emit yellowUpdated();
    }
    else if(cmd == "rpc_nodes") {
        this->onWSNodes(msg.value("data").toArray());
    }
    else if(cmd == "xmrig") {
        this->XMRigDownloads(msg.value("data").toObject());
    }
    else if(cmd == "wownerod_releases") {
        emit WownerodDownloads(msg.value("data").toObject());
    }
    else if(cmd == "crypto_rates") {
        QJsonArray crypto_rates = msg.value("data").toArray();
        AppContext::prices->cryptoPricesReceived(crypto_rates);
    }
    else if(cmd == "fiat_rates") {
        QJsonObject fiat_rates = msg.value("data").toObject();
        AppContext::prices->fiatPricesReceived(fiat_rates);
    }
    else if(cmd == "reddit") {
        QJsonArray reddit_data = msg.value("data").toArray();
        this->onWSReddit(reddit_data);
    }
    else if(cmd == "forum") {
        QJsonArray forum_data = msg.value("data").toArray();
        this->onWSForum(forum_data);
    }
    else if(cmd == "funding_proposals") {
        auto ccs_data = msg.value("data").toArray();
        this->onWSCCS(ccs_data);
    }
    else if(cmd == "suchwow") {
        QJsonArray such_data = msg.value("data").toArray();
        emit suchWowUpdated(such_data);
    }
    else if(cmd == "txFiatHistory") {
        auto txFiatHistory_data = msg.value("data").toObject();
        AppContext::txFiatHistory->onWSData(txFiatHistory_data);
    }
    else if(cmd == "wowlet_releases") {
        versionPending = msg.value("data").toObject();
        auto version_str = versionPending.value("version").toString();

        if(Utils::versionOutdated(WOWLET_VERSION_SEMVER, version_str))
            emit versionOutdated(version_str, versionPending);
    }
    else if(cmd == "kill") {
        // used *only* in dire emergencies
        auto killme = msg.value("data").toBool();
        if(killme)
            QCoreApplication::quit();
    }
#if defined(HAS_OPENVR)
    else if(cmd == "requestPIN") {
        auto pin = msg.value("data").toString();
        emit pinReceived(pin);
    }

    else if(cmd == "lookupPIN") {
        auto lookup_data = msg.value("data").toObject();
        auto address = lookup_data.value("address").toString();
        auto pin = lookup_data.value("PIN").toString();

        if(address.isEmpty())
            emit pinLookupErrorReceived();
        else
            emit pinLookupReceived(address, pin);
    }
#endif
}

void AppContext::onWSNodes(const QJsonArray &nodes) {
    QList<QSharedPointer<WowletNode>> l;
    for (auto &&entry: nodes) {
        auto obj = entry.toObject();
        auto nettype = obj.value("nettype");
        auto type = obj.value("type");

        // filter remote node network types
        if(nettype == "mainnet" && this->networkType != NetworkType::MAINNET)
            continue;
        if(nettype == "stagenet" && this->networkType != NetworkType::STAGENET)
            continue;
        if(nettype == "testnet" && this->networkType != NetworkType::TESTNET)
            continue;

        if(type == "clearnet" && (this->isTails || this->isWhonix || this->isTorSocks))
            continue;
        if(type == "tor" && (!(this->isTails || this->isWhonix || this->isTorSocks)))
            continue;

        auto node = new WowletNode(
                obj.value("address").toString(),
                 obj.value("height").toInt(),
                 obj.value("target_height").toInt(),
                obj.value("online").toBool());
        QSharedPointer<WowletNode> r = QSharedPointer<WowletNode>(node);
        l.append(r);
    }
    this->nodes->onWSNodesReceived(l);
}

void AppContext::onWSForum(const QJsonArray& forum_data) {
    QList<QSharedPointer<ForumPost>> l;

    for (auto &&entry: forum_data) {
        auto obj = entry.toObject();
        auto forumPost = new ForumPost(
                obj.value("title").toString(),
                obj.value("author").toString(),
                obj.value("permalink").toString(),
                obj.value("comments").toInt());
        QSharedPointer<ForumPost> r = QSharedPointer<ForumPost>(forumPost);
        l.append(r);
    }

    emit forumUpdated(l);
}

void AppContext::onWSReddit(const QJsonArray& reddit_data) {
    QList<QSharedPointer<RedditPost>> l;

    for (auto &&entry: reddit_data) {
        auto obj = entry.toObject();
        auto redditPost = new RedditPost(
                obj.value("title").toString(),
                obj.value("author").toString(),
                obj.value("permalink").toString(),
                obj.value("comments").toInt());
        QSharedPointer<RedditPost> r = QSharedPointer<RedditPost>(redditPost);
        l.append(r);
    }

    emit redditUpdated(l);
}

void AppContext::onWSCCS(const QJsonArray &ccs_data) {
    QList<QSharedPointer<CCSEntry>> l;


    QStringList fonts = {"state", "address", "author", "date",
                         "title", "target_amount", "raised_amount",
                         "percentage_funded", "contributions"};

    for (auto &&entry: ccs_data) {
        auto obj = entry.toObject();
        auto c = QSharedPointer<CCSEntry>(new CCSEntry());

        if (obj.value("state").toString() != "FUNDING-REQUIRED")
            continue;

        c->state = obj.value("state").toString();
        c->address = obj.value("address").toString();
        c->author = obj.value("author").toString();
        c->date = obj.value("date").toString();
        c->title = obj.value("title").toString();
        c->url = obj.value("url").toString();
        c->target_amount = obj.value("target_amount").toDouble();
        c->raised_amount = obj.value("raised_amount").toDouble();
        c->percentage_funded = obj.value("percentage_funded").toDouble();
        c->contributions = obj.value("contributions").toInt();
        l.append(c);
    }

    emit ccsUpdated(l);
}

void AppContext::createConfigDirectory(const QString &dir) {
    auto config_dir_tor = QString("%1%2").arg(dir).arg("tor");
    auto config_dir_tordata = QString("%1%2").arg(dir).arg("tor/data");

    QStringList createDirs({dir, config_dir_tor, config_dir_tordata});
    for(const auto &d: createDirs) {
        if(!Utils::dirExists(d)) {
            qDebug() << QString("Creating directory: %1").arg(d);
            if (!QDir().mkpath(d))
                throw std::runtime_error("Could not create directory " + d.toStdString());
        }
    }

#ifdef HAS_OPENVR
    auto config_dir_vr = QString("%1%2").arg(dir, "vr");
    if(!Utils::dirExists(config_dir_vr)) {
        qDebug() << QString("Creating directory: %1").arg(config_dir_vr);
        if (!QDir().mkpath(config_dir_vr))
            throw std::runtime_error("Could not create directory " + config_dir_vr.toStdString());
    }
#endif
}

void AppContext::createWalletWithoutSpecifyingSeed(const QString &name, const QString &password) {
    WowletSeed seed = WowletSeed(this->restoreHeights[this->networkType], this->coinName, this->seedLanguage);
    auto path = QDir(this->defaultWalletDir).filePath(name);
    this->createWallet(seed, path, password);
}

void AppContext::createWallet(WowletSeed seed, const QString &path, const QString &password) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(seed.mnemonic.isEmpty()) {
        emit walletCreatedError("Mnemonic seed error. Failed to write wallet.");
        return;
    }

    Wallet *wallet = nullptr;
    if (seed.seedType == SeedType::TEVADOR) {
        wallet = this->walletManager->createDeterministicWalletFromSpendKey(path, password, seed.language, this->networkType, seed.spendKey, seed.restoreHeight, this->kdfRounds);
        wallet->setCacheAttribute("wowlet.seed", seed.mnemonic.join(" "));
    }
    if (seed.seedType == SeedType::MONERO) {
        wallet = this->walletManager->recoveryWallet(path, password, seed.mnemonic.join(" "), "", this->networkType, seed.restoreHeight, this->kdfRounds);
    }

    this->currentWallet = wallet;
    if(this->currentWallet == nullptr) {
        emit walletCreatedError("Failed to write wallet");
        return;
    }

    this->createWalletFinish(password);
}

void AppContext::createWalletViewOnly(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight) {
    if(Utils::fileExists(path)) {
        auto err = QString("Failed to write wallet to path: \"%1\"; file already exists.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!this->walletManager->addressValid(address, this->networkType)) {
        auto err = QString("Failed to create wallet. Invalid address provided.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!this->walletManager->keyValid(viewkey, address, true, this->networkType)) {
        auto err = QString("Failed to create wallet. Invalid viewkey provided.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    if(!spendkey.isEmpty() && !this->walletManager->keyValid(spendkey, address, false, this->networkType)) {
        auto err = QString("Failed to create wallet. Invalid spendkey provided.").arg(path);
        qCritical() << err;
        emit walletCreatedError(err);
        return;
    }

    this->currentWallet = this->walletManager->createWalletFromKeys(path, this->seedLanguage, this->networkType, address, viewkey, spendkey, restoreHeight);
    this->createWalletFinish(password);
}

void AppContext::createWalletFinish(const QString &password) {
    this->currentWallet->setPassword(password);
    this->currentWallet->store();
    this->walletPassword = password;
    emit walletCreated(this->currentWallet);

    // emit signal on behalf of walletManager, open wallet
    this->walletManager->walletOpened(this->currentWallet);
}

void AppContext::initRestoreHeights() {
    restoreHeights[NetworkType::TESTNET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_wownero_mainnet.txt", NetworkType::TESTNET);
    restoreHeights[NetworkType::STAGENET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_wownero_mainnet.txt", NetworkType::STAGENET);
    restoreHeights[NetworkType::MAINNET] = RestoreHeightLookup::fromFile(":/assets/restore_heights_wownero_mainnet.txt", NetworkType::MAINNET);
}

void AppContext::onSetRestoreHeight(quint64 height){
    auto seed = this->currentWallet->getCacheAttribute("wowlet.seed");
    if(!seed.isEmpty()) {
        const auto msg = "This wallet has a 14 word mnemonic seed which has the restore height embedded.";
        emit setRestoreHeightError(msg);
        return;
    }

    this->currentWallet->setWalletCreationHeight(height);
    this->currentWallet->setPassword(this->currentWallet->getPassword());  // trigger .keys write

    // nuke wallet cache
    const auto fn = this->currentWallet->path();
    this->walletManager->clearWalletCache(fn);

    emit customRestoreHeightSet(height);
}

void AppContext::onOpenAliasResolve(const QString &openAlias) {
    // @TODO: calling this freezes for about 1-2 seconds :/
    const auto result = this->walletManager->resolveOpenAlias(openAlias);
    const auto spl = result.split("|");
    auto msg = QString("");
    if(spl.count() != 2) {
        msg = "Internal error";
        emit openAliasResolveError(msg);
        return;
    }

    const auto &status = spl.at(0);
    const auto &address = spl.at(1);
    const auto valid = this->walletManager->addressValid(address, this->networkType);
    if(status == "false"){
        if(valid){
            msg = "Address found, but the DNSSEC signatures could not be verified, so this address may be spoofed";
            emit openAliasResolveError(msg);
            return;
        } else {
            msg = "No valid address found at this OpenAlias address, but the DNSSEC signatures could not be verified, so this may be spoofed";
            emit openAliasResolveError(msg);
            return;
        }
    } else if(status != "true") {
        msg = "Internal error";
        emit openAliasResolveError(msg);
        return;
    }

    if(valid){
        emit openAliasResolved(address, openAlias);
        return;
    }

    msg = QString("Address validation error.");
    if(!address.isEmpty())
        msg += QString(" Perhaps it is of the wrong network type."
                       "\n\nOpenAlias: %1\nAddress: %2").arg(openAlias).arg(address);
    emit openAliasResolveError(msg);
}

void AppContext::donateBeg() {
    if(this->currentWallet == nullptr) return;
    if(this->networkType != NetworkType::Type::MAINNET) return;
    if(this->currentWallet->viewOnly()) return;

    auto donationCounter = config()->get(Config::donateBeg).toInt();
    if(donationCounter == -1)
        return;  // previously donated

    donationCounter += 1;
    if (donationCounter % m_donationBoundary == 0)
        emit donationNag();
    config()->set(Config::donateBeg, donationCounter);
}

AppContext::~AppContext() {}

// ############################################## LIBWALLET QT #########################################################

void AppContext::onMoneySpent(const QString &txId, quint64 amount) {
    auto amount_num = amount / globals::cdiv;
    qDebug() << Q_FUNC_INFO << txId << " " << QString::number(amount_num);
}

void AppContext::onMoneyReceived(const QString &txId, quint64 amount) {
    // Incoming tx included in a block.
    auto amount_num = amount / globals::cdiv;
    qDebug() << Q_FUNC_INFO << txId << " " << QString::number(amount_num);
}

void AppContext::onUnconfirmedMoneyReceived(const QString &txId, quint64 amount) {
    // Incoming transaction in pool
    auto amount_num = amount / globals::cdiv;
    qDebug() << Q_FUNC_INFO << txId << " " << QString::number(amount_num);

    if(this->currentWallet->synchronized()) {
        auto notify = QString("%1 WOW (pending)").arg(amount_num);
        Utils::desktopNotify("Payment received", notify, 5000);
    }
}

void AppContext::onWalletUpdate() {
    if (this->currentWallet->synchronized()) {
        this->refreshModels();
        this->storeWallet();
    }

    this->updateBalance();
}

void AppContext::onWalletRefreshed(bool success) {
    if (!this->refreshed) {
        refreshModels();
        this->refreshed = true;
        emit walletRefreshed();
        // store wallet immediately upon finishing synchronization
        this->currentWallet->store();
    }

    qDebug() << "Wallet refresh status: " << success;

    this->currentWallet->refreshHeightAsync();
}

void AppContext::onWalletNewBlock(quint64 blockheight, quint64 targetHeight) {
    this->syncStatusUpdated(blockheight, targetHeight);

    if (this->currentWallet->synchronized()) {
        this->currentWallet->coins()->refreshUnlocked();
        this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
        // Todo: only refresh tx confirmations
    }
}

void AppContext::onHeightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight) {
    qDebug() << Q_FUNC_INFO << walletHeight << daemonHeight << targetHeight;

    if (this->currentWallet->connectionStatus() == Wallet::ConnectionStatus_Disconnected)
        return;

    if (daemonHeight < targetHeight) {
        emit blockchainSync(daemonHeight, targetHeight);
    }
    else {
        this->syncStatusUpdated(walletHeight, daemonHeight);
    }
}

void AppContext::onTransactionCreated(PendingTransaction *tx, const QVector<QString> &address) {
    for (auto &addr : address) {
        if (addr == this->donationAddress) {
            this->donationSending = true;
        }
    }

    // Let UI know that the transaction was constructed
    emit endTransaction();

    // Some validation
    auto tx_status = tx->status();
    auto err = QString("Can't create transaction: ");

    if(tx_status != PendingTransaction::Status_Ok){
        auto tx_err = tx->errorString();
        qCritical() << tx_err;

        if (this->currentWallet->connectionStatus() == Wallet::ConnectionStatus_WrongVersion)
            err = QString("%1 Wrong daemon version: %2").arg(err).arg(tx_err);
        else
            err = QString("%1 %2").arg(err).arg(tx_err);

        qDebug() << Q_FUNC_INFO << err;
        emit createTransactionError(err);
        this->currentWallet->disposeTransaction(tx);
        return;
    } else if (tx->txCount() == 0) {
        err = QString("%1 %2").arg(err).arg("No unmixable outputs to sweep.");
        qDebug() << Q_FUNC_INFO << err;
        emit createTransactionError(err);
        this->currentWallet->disposeTransaction(tx);
        return;
    }

    // tx created, but not sent yet. ask user to verify first.
    emit createTransactionSuccess(tx, address);

    if(this->autoCommitTx) {
        this->currentWallet->commitTransactionAsync(tx);
    }
}

QString AppContext::getAddress(quint32 accountIndex, quint32 addressIndex) {
    return this->currentWallet->address(accountIndex, addressIndex);
}

void AppContext::onAskReceivingPIN() {
    // request new receiving PIN from wowlet-backend
    if(this->currentWallet == nullptr)
        return;

    auto address = this->currentWallet->address(0, 1);
    QString signature = this->currentWallet->signMessage(address, false, address);

    QJsonObject data;
    data["signature"] = signature;
    data["address"] = address;

    QJsonObject obj;
    obj["cmd"] = "requestPIN";
    obj["data"] = data;

    QJsonDocument doc = QJsonDocument(obj);
    this->ws->sendMsg(doc.toJson(QJsonDocument::Compact));
}

void AppContext::onLookupReceivingPIN(QString pin) {
    // lookup PIN -> address
    if(this->currentWallet == nullptr)
        return;

    auto address = this->currentWallet->address(0, 1);
    QString signature = this->currentWallet->signMessage(address, false, address);

    QJsonObject data;
    data["PIN"] = pin;

    QJsonObject obj;
    obj["cmd"] = "lookupPIN";
    obj["data"] = data;

    QJsonDocument doc = QJsonDocument(obj);
    this->ws->sendMsg(doc.toJson(QJsonDocument::Compact));
}

void AppContext::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid){
    this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->coins()->refresh(this->currentWallet->currentSubaddressAccount());

    // Store wallet immediately so we don't risk losing tx key if wallet crashes
    this->currentWallet->store();

    this->updateBalance();

    emit transactionCommitted(status, tx, txid);

    // this tx was a donation to WOWlet, stop our nagging
    if(this->donationSending) {
        this->donationSending = false;
        config()->set(Config::donateBeg, -1);
    }
}

void AppContext::storeWallet() {
    // Do not store a synchronizing wallet: store() is NOT thread safe and may crash the wallet
    if (this->currentWallet == nullptr || !this->currentWallet->synchronized())
        return;

    qDebug() << "Storing wallet";
    this->currentWallet->store();
}

void AppContext::updateBalance() {
    if (!this->currentWallet)
        return;

    quint64 balance_u = this->currentWallet->balance();
    AppContext::balance = balance_u / globals::cdiv;
    double spendable = this->currentWallet->unlockedBalance();

    // formatted
    QString fmt_str = QString("Balance: %1 WOW").arg(Utils::balanceFormat(spendable));
    if (balance > spendable)
        fmt_str += QString(" (+%1 WOW unconfirmed)").arg(Utils::balanceFormat(balance - spendable));

    emit balanceUpdated(balance_u, spendable);
    emit balanceUpdatedFormatted(fmt_str);
}

void AppContext::syncStatusUpdated(quint64 height, quint64 target) {
    if (height < (target - 1)) {
        emit refreshSync(height, target);
    }
    else {
        this->updateBalance();
        emit synchronized();
    }
}

void AppContext::refreshModels() {
    if (!this->currentWallet)
        return;

    this->currentWallet->history()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->subaddress()->refresh(this->currentWallet->currentSubaddressAccount());
    this->currentWallet->coins()->refresh(this->currentWallet->currentSubaddressAccount());
    // Todo: set timer for refreshes
}

void AppContext::setupPathsUnix() {
    this->defaultWalletDir = QString("%1/Wownero/wallets").arg(this->configRoot);
    this->defaultWalletDirRoot = QString("%1/Wownero").arg(this->configRoot);
}

void AppContext::setupPathsWindows() {
    this->defaultWalletDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Wownero";
    this->defaultWalletDirRoot = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void AppContext::setupPathsAndroid() {
    this->defaultWalletDir = QString("%1/Wownero/wallets").arg(this->pathGenericData);
    this->defaultWalletDirRoot = QString("%1/Wownero").arg(this->pathGenericData);
}

void AppContext::setupPathsTails() {
    QString portablePath = []{
        QString appImagePath = qgetenv("APPIMAGE");
        if (appImagePath.isEmpty()) {
            qDebug() << "Not an appimage, using currentPath()";
            return QDir::currentPath() + "/.wowlet";
        }

        QFileInfo appImageDir(appImagePath);
        return appImageDir.absoluteDir().path() + "/.wowlet";
    }();

    if (QDir().mkpath(portablePath)) {
        this->configRoot = portablePath;
    } else {
        qCritical() << "Unable to create portable directory: " << portablePath;
    }
}

