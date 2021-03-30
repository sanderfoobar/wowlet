// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_DEBUGINFODIALOG_H
#define WOWLET_DEBUGINFODIALOG_H

#include <QDialog>
#include "appcontext.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class DebugInfoDialog;
}

class DebugInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DebugInfoDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~DebugInfoDialog() override;

private:
    QString statusToString(Wallet::ConnectionStatus status);
    void copyToClipboad();
    void updateInfo();

    QTimer m_updateTimer;
    AppContext *m_ctx;

    Ui::DebugInfoDialog *ui;
};

#endif //WOWLET_DEBUGINFODIALOG_H
