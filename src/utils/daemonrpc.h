// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_DAEMON_RPC_H
#define WOWLET_DAEMON_RPC_H

#include <QObject>

#include "utils/networking.h"

class DaemonRpc : public QObject {
    Q_OBJECT

public:
    enum Endpoint {
        SEND_RAW_TRANSACTION = 0,
        GET_TRANSACTIONS
    };

    struct DaemonResponse {
        explicit DaemonResponse(bool ok, Endpoint endpoint, QString status, QJsonObject obj = {})
                : ok(ok), endpoint(endpoint), status(std::move(status)), obj(std::move(obj)) {};

        bool ok;
        DaemonRpc::Endpoint endpoint;
        QString status;
        QJsonObject obj;
    };

    explicit DaemonRpc(QObject *parent, UtilsNetworking *network, QString daemonAddress);

    void sendRawTransaction(const QString &tx_as_hex, bool do_not_relay = false, bool do_sanity_checks = true);
    void getTransactions(const QStringList &txs_hashes, bool decode_as_json = false, bool prune = false);

    void setDaemonAddress(const QString &daemonAddress);
    void setNetwork(UtilsNetworking *network);

signals:
    void ApiResponse(DaemonResponse resp);

private slots:
    void onResponse(QNetworkReply *reply, Endpoint endpoint);
    QString onSendRawTransactionFailed(const QJsonObject &obj);

private:
    UtilsNetworking *m_network;
    QString m_daemonAddress;
};


#endif //WOWLET_DAEMON_RPC_H
