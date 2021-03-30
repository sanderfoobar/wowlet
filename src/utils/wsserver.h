// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WSSERVER_H
#define FEATHER_WSSERVER_H


#include <QObject>
#include <QList>
#include <QtNetwork>

#include "appcontext.h"
#include "utils/keysfiles.h"
#include "qrcode/QrCode.h"

#include "libwalletqt/WalletManager.h"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WSServer : public QObject
{
Q_OBJECT
public:
    explicit WSServer(AppContext *ctx, const QHostAddress &host, const quint16 port, const QString &password, bool debug = false, QObject *parent = nullptr);
    ~WSServer();

signals:
    void closed();

private slots:
    void onNewConnection();
    void processBinaryMessage(QByteArray buffer);
    void socketDisconnected();

    // libwalletqt
    void onBalanceUpdated(quint64 balance, quint64 spendable);
    void onSynchronized();
    void onWalletOpened(Wallet *wallet);
    void onWalletClosed();
    void onConnectionStatusChanged(int status);
    void onCreateTransactionError(const QString &message);
    void onCreateTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address);
    void onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);
    void onBlockchainSync(int height, int target);
    void onRefreshSync(int height, int target);
    void onWalletOpenedError(const QString &err);
    void onWalletCreatedError(const QString &err);
    void onWalletCreated(Wallet *wallet);
    void onWalletOpenPasswordRequired(bool invalidPassword, const QString &path);
    void onInitiateTransaction();

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QMap<QString, bool> m_clients_auth;
    bool m_debug;
    QString m_walletDir;
    AppContext *m_ctx;
    QString m_password;

    QString connectionId(QWebSocket *pSocket);

    QByteArray createWSMessage(const QString &cmd, const QJsonObject &obj);
    QByteArray createWSMessage(const QString &cmd, const QJsonArray &arr);
    QByteArray createWSMessage(const QString &cmd, const int val);
    QByteArray createWSMessage(const QString &cmd, const QString &val);
    void sendAll(const QString &cmd, const QJsonArray &arr);
    void sendAll(const QString &cmd, const QJsonObject &obj);
    void sendAll(const QString &cmd, int val);
    void sendAll(const QString &cmd, const QString &val);
};

#endif //FEATHER_WSSERVER_H
