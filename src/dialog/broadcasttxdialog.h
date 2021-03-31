// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_BROADCASTTXDIALOG_H
#define WOWLET_BROADCASTTXDIALOG_H

#include <QDialog>
#include "appcontext.h"
#include "utils/daemonrpc.h"

namespace Ui {
    class BroadcastTxDialog;
}

class BroadcastTxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BroadcastTxDialog(QWidget *parent, AppContext *ctx, const QString &transactionHex = "");
    ~BroadcastTxDialog() override;

private slots:
    void broadcastTx();
    void onApiResponse(const DaemonRpc::DaemonResponse &resp);

private:
    Ui::BroadcastTxDialog *ui;
    UtilsNetworking *m_network;
    AppContext *m_ctx;
    DaemonRpc *m_rpc;
};


#endif //WOWLET_BROADCASTTXDIALOG_H
