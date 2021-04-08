// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QtNetwork>
#include <QScreen>
#include <QMessageBox>
#include <QtNetwork>
#include <QClipboard>
#include <QCompleter>
#include <QDesktopServices>
#include <QTextCodec>

#include <model/SubaddressModel.h>
#include <model/SubaddressProxyModel.h>
#include <model/CoinsModel.h>
#include <model/CoinsProxyModel.h>
#include "model/AddressBookModel.h"
#include "model/TransactionHistoryModel.h"
#include "libwalletqt/AddressBook.h"
#include "libwalletqt/TransactionHistory.h"
#include <globals.h>

#include "wsserver.h"
#include "appcontext.h"
#include "utils/utils.h"

WSServer::WSServer(AppContext *ctx, const QHostAddress &host, const quint16 port, const QString &password, bool debug, QObject *parent) :
        QObject(parent),
        m_debug(debug),
        m_ctx(ctx),
        m_password(password),
        m_pWebSocketServer(
                new QWebSocketServer(QStringLiteral("WOWlet Daemon WS"),
                                     QWebSocketServer::NonSecureMode, this)) {
    if (!m_pWebSocketServer->listen(QHostAddress::Any, port))
        return;

    // turn on auto tx commits
    ctx->autoCommitTx = true;

    qDebug() << "websocket server listening on port" << port;

    connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
    connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WSServer::closed);

    connect(m_ctx, &AppContext::walletClosed, this, &WSServer::onWalletClosed);
    connect(m_ctx, &AppContext::balanceUpdated, this, &WSServer::onBalanceUpdated);
    connect(m_ctx, &AppContext::walletOpened, this, &WSServer::onWalletOpened);
    connect(m_ctx, &AppContext::walletOpenedError, this, &WSServer::onWalletOpenedError);
    connect(m_ctx, &AppContext::walletCreatedError, this, &WSServer::onWalletCreatedError);
    connect(m_ctx, &AppContext::walletCreated, this, &WSServer::onWalletCreated);
    connect(m_ctx, &AppContext::synchronized, this, &WSServer::onSynchronized);
    connect(m_ctx, &AppContext::blockchainSync, this, &WSServer::onBlockchainSync);
    connect(m_ctx, &AppContext::refreshSync, this, &WSServer::onRefreshSync);
    connect(m_ctx, &AppContext::createTransactionError, this, &WSServer::onCreateTransactionError);
    connect(m_ctx, &AppContext::createTransactionSuccess, this, &WSServer::onCreateTransactionSuccess);
    connect(m_ctx, &AppContext::transactionCommitted, this, &WSServer::onTransactionCommitted);
    connect(m_ctx, &AppContext::walletOpenPasswordNeeded, this, &WSServer::onWalletOpenPasswordRequired);
    connect(m_ctx, &AppContext::initiateTransaction, this, &WSServer::onInitiateTransaction);

    m_walletDir = m_ctx->defaultWalletDir;

    // Bootstrap Tor/websockets
    m_ctx->initTor();
    m_ctx->initWS();
}

QString WSServer::connectionId(QWebSocket *pSocket) {
    return QString("%1#%2").arg(pSocket->peerAddress().toString()).arg(pSocket->peerPort());
}

void WSServer::onNewConnection() {
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WSServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WSServer::socketDisconnected);

    m_clients << pSocket;
    m_clients_auth[this->connectionId(pSocket)] = false;

    // blast wallet listing on connect
    QJsonArray arr;
    for(const QVariant &wallet: m_ctx->listWallets())
        arr << wallet.value<WalletKeysFiles>().toJsonObject();
    auto welcomeWalletMessage = WSServer::createWSMessage("walletList", arr);
    pSocket->sendBinaryMessage(welcomeWalletMessage);

    // and the current state of appcontext
    QJsonObject obj;

    if(this->m_ctx->currentWallet == nullptr) {
        obj["state"] = "walletClosed";
    }
    else {
        obj["state"] = "walletOpened";
        obj["walletPath"] = m_ctx->currentWallet->path();
    }
    this->sendAll("state", obj);
}

void WSServer::processBinaryMessage(QByteArray buffer) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    const QString cid = this->connectionId(pClient);

    if (m_debug)
        qDebug() << "Websocket (server) received:" << buffer;
    if (!pClient)
        return;

    QJsonDocument doc = QJsonDocument::fromJson(buffer);
    QJsonObject object = doc.object();

    QString cmd = object.value("cmd").toString();

    if(m_clients_auth.contains(cid) && !m_clients_auth[cid]) {
        if (cmd == "password") {
            auto data = object.value("data").toObject();
            auto passwd = data.value("password").toString();
            if(passwd != this->m_password) {
                this->sendAll("passwordIncorrect", "authentication failed.");
                return;
            } else {
                this->m_clients_auth[cid] = true;
                this->sendAll("passwordSuccess", "authentication OK.");
                return;
            }
        } else {
            this->sendAll("passwordIncorrect", "authentication failed.");
            return;
        }
    }

    if(cmd == "openWallet") {
        auto data = object.value("data").toObject();
        auto path = data.value("path").toString();
        auto passwd = data.value("password").toString();

        m_ctx->onOpenWallet(path, passwd);
    } else if (cmd == "closeWallet") {
        if (m_ctx->currentWallet == nullptr)
            return;

        m_ctx->closeWallet(true, true);
    } else if(cmd == "addressList") {
        auto data = object.value("data").toObject();
        auto accountIndex = data.value("accountIndex").toInt();
        auto addressIndex = data.value("addressIndex").toInt();

        auto limit = data.value("limit").toInt(50);
        auto offset = data.value("offset").toInt(0);

        QJsonArray arr;
        for(int i = offset; i != limit; i++) {
            arr << m_ctx->currentWallet->address((quint32) accountIndex, (quint32) addressIndex + i);
        }

        QJsonObject obj;
        obj["accountIndex"] = accountIndex;
        obj["addressIndex"] = addressIndex;
        obj["offset"] = offset;
        obj["limit"] = limit;
        obj["addresses"] = arr;
        this->sendAll("addressList", arr);
    } else if(cmd == "sendTransaction") {
        auto data = object.value("data").toObject();
        auto address = data.value("address").toString();
        auto amount = data.value("amount").toDouble(0);
        auto description = data.value("description").toString();
        bool all = data.value("all").toBool(false);

        if(!WalletManager::addressValid(address, m_ctx->currentWallet->nettype())){
            this->sendAll("transactionError", "Could not validate address");
            return;
        }

        if(amount <= 0) {
            this->sendAll("transactionError", "y u send 0");
            return;
        }

        m_ctx->onCreateTransaction(address, (quint64) amount, description, all);
    } else if(cmd == "createWallet") {
        auto data = object.value("data").toObject();

        auto name = data.value("name").toString();
        auto path = data.value("path").toString();
        auto password = data.value("password").toString();
        QString walletPath;

        if(name.isEmpty()){
            this->sendAll("walletCreatedError", "Supply a name for your wallet");
            return;
        }

        if(path.isEmpty()) {
            walletPath = QDir(m_walletDir).filePath(name + ".keys");
            if(Utils::fileExists(walletPath)) {
                auto err = QString("Filepath already exists: %1").arg(walletPath);
                this->sendAll("walletCreatedError", err);
                return;
            }
        }

        WowletSeed seed = WowletSeed(m_ctx->restoreHeights[m_ctx->networkType], m_ctx->coinName, m_ctx->seedLanguage);
        m_ctx->createWallet(seed, walletPath, password);
    } else if(cmd == "transactionHistory") {
        m_ctx->currentWallet->history()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
        auto *model = m_ctx->currentWallet->history();

        QJsonArray arr = model->toJsonArray();
        this->sendAll("transactionHistory", arr);
    } else if (cmd == "addressBook") {
        QJsonArray arr = m_ctx->currentWallet->addressBookModel()->toJsonArray();
        this->sendAll("addressBook", arr);
    }
}

void WSServer::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString cid = connectionId(pClient);

    m_clients_auth[cid] = false;

    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

// templates are forbidden!
QByteArray WSServer::createWSMessage(const QString &cmd, const QJsonArray &arr) {
    QJsonObject jsonObject = QJsonObject();
    jsonObject["cmd"] = cmd;
    jsonObject["data"] = arr;
    QJsonDocument doc = QJsonDocument(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}
QByteArray WSServer::createWSMessage(const QString &cmd, const QJsonObject &obj) {
    QJsonObject jsonObject = QJsonObject();
    jsonObject["cmd"] = cmd;
    jsonObject["data"] = obj;
    QJsonDocument doc = QJsonDocument(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray WSServer::createWSMessage(const QString &cmd, const int val) {
    QJsonObject jsonObject = QJsonObject();
    jsonObject["cmd"] = cmd;
    jsonObject["data"] = val;
    QJsonDocument doc = QJsonDocument(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray WSServer::createWSMessage(const QString &cmd, const QString &val) {
    QJsonObject jsonObject = QJsonObject();
    jsonObject["cmd"] = cmd;
    jsonObject["data"] = val;
    QJsonDocument doc = QJsonDocument(jsonObject);
    return doc.toJson(QJsonDocument::Compact);
}

WSServer::~WSServer() {
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void WSServer::sendAll(const QString &cmd, const QJsonObject &obj) {
    for(QWebSocket *pSocket: m_clients) {
        pSocket->sendBinaryMessage(WSServer::createWSMessage(cmd, obj));
    }
}

void WSServer::sendAll(const QString &cmd, const QJsonArray &arr) {
    for(QWebSocket *pSocket: m_clients) {
        pSocket->sendBinaryMessage(WSServer::createWSMessage(cmd, arr));
    }
}

void WSServer::sendAll(const QString &cmd, int val) {
    for(QWebSocket *pSocket: m_clients) {
        pSocket->sendBinaryMessage(WSServer::createWSMessage(cmd, val));
    }
}

void WSServer::sendAll(const QString &cmd, const QString &val) {
    for(QWebSocket *pSocket: m_clients) {
        pSocket->sendBinaryMessage(WSServer::createWSMessage(cmd, val));
    }
}

// ======================================================================

void WSServer::onWalletOpened(Wallet *wallet) {
    connect(m_ctx->currentWallet, &Wallet::connectionStatusChanged, this, &WSServer::onConnectionStatusChanged);

    auto obj = wallet->toJsonObject();
    sendAll("walletOpened", obj);
}

void WSServer::onBlockchainSync(int height, int target) {
    QJsonObject obj;
    obj["height"] = height;
    obj["target"] = target;
    sendAll("blockchainSync", obj);
}

void WSServer::onRefreshSync(int height, int target) {
    QJsonObject obj;
    obj["height"] = height;
    obj["target"] = target;
    sendAll("refreshSync", obj);
}

void WSServer::onWalletClosed() {
    QJsonObject obj;
    sendAll("walletClosed", obj);
}

void WSServer::onBalanceUpdated(quint64 balance, quint64 spendable) {
    QJsonObject obj;
    obj["balance"] = balance / globals::cdiv;
    obj["spendable"] = spendable / globals::cdiv;
    sendAll("balanceUpdated", obj);
}

void WSServer::onWalletOpenedError(const QString &err) {
    sendAll("walletOpenedError", err);
}

void WSServer::onWalletCreatedError(const QString &err) {
    sendAll("walletCreatedError", err);
}

void WSServer::onWalletCreated(Wallet *wallet) {
    auto obj = wallet->toJsonObject();
    sendAll("walletCreated", obj);
}

void WSServer::onSynchronized() {
    QJsonObject obj;
    sendAll("synchronized", obj);
}

void WSServer::onWalletOpenPasswordRequired(bool invalidPassword, const QString &path) {
    QJsonObject obj;
    obj["invalidPassword"] = invalidPassword;
    obj["path"] = path;
    sendAll("walletOpenPasswordRequired", obj);
}

void WSServer::onConnectionStatusChanged(int status) {
    sendAll("connectionStatusChanged", status);
}

void WSServer::onInitiateTransaction() {
    QJsonObject obj;
    sendAll("transactionStarted", obj);
}

void WSServer::onCreateTransactionError(const QString &message) {
    sendAll("transactionError", message);
}

void WSServer::onCreateTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address) {

}

void WSServer::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList &txid) {
    QString preferredCur = config()->get(Config::preferredFiatCurrency).toString();

    auto convert = [preferredCur](double amount){
        return QString::number(AppContext::prices->convert("WOW", preferredCur, amount), 'f', 2);
    };

    QJsonObject obj;
    QJsonArray txids;

    for(const QString &id: txid)
        txids << id;

    obj["txid"] = txids;
    obj["status"] = status;
    obj["amount"] = tx->amount() / globals::cdiv;
    obj["fee"] = tx->fee() / globals::cdiv;
    obj["total"] = (tx->amount() + tx->fee()) / globals::cdiv;

    obj["amount_fiat"] = convert(tx->amount() / globals::cdiv);
    obj["fee_fiat"] = convert(tx->fee() / globals::cdiv);
    obj["total_fiat"] = convert((tx->amount() + tx->fee()) / globals::cdiv);

    sendAll("transactionSent", obj);
}