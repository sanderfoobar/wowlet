// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_TXIMPORTDIALOG_H
#define WOWLET_TXIMPORTDIALOG_H

#include <QDialog>
#include "appcontext.h"
#include "utils/daemonrpc.h"

namespace Ui {
    class TxImportDialog;
}

class TxImportDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxImportDialog(QWidget *parent, AppContext *ctx);
    ~TxImportDialog() override;

private slots:
    void loadTx();
    void onImport();
    void onApiResponse(const DaemonRpc::DaemonResponse &resp);

private:
    Ui::TxImportDialog *ui;
    UtilsNetworking *m_network;
    AppContext *m_ctx;
    DaemonRpc *m_rpc;
    QTimer *m_loadTimer;

    QJsonObject m_transaction;
};


#endif //WOWLET_TXIMPORTDIALOG_H
