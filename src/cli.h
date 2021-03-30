// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_CLI_H
#define FEATHER_CLI_H

#include <QtCore>
#include "appcontext.h"
#include <utils/wsserver.h>

enum CLIMode {
    CLIModeExportContacts,
    CLIModeExportTxHistory,
    CLIDaemonize
};

class CLI : public QObject
{
    Q_OBJECT
public:
    CLIMode mode;
    explicit CLI(AppContext *ctx, QObject *parent = nullptr);
    ~CLI() override;

    QString backgroundWebsocketAddress;
    quint16 backgroundWebsocketPort;
    QString backgroundWebsocketPassword;

public slots:
    void run();

    //libwalletqt
    void onWalletOpened();
    void onWalletOpenedError(const QString& err);
    void onWalletOpenPasswordRequired(bool invalidPassword, const QString &path);

private:
    AppContext *ctx;
    WSServer *m_wsServer;

private slots:
    void finished(const QString &msg);
    void finishedError(const QString &err);

signals:
    void closeApplication();
};

#endif //FEATHER_CLI_H
