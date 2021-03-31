// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_WALLETINFODIALOG_H
#define WOWLET_WALLETINFODIALOG_H

#include <QDialog>

#include "appcontext.h"

namespace Ui {
    class WalletInfoDialog;
}

class WalletInfoDialog : public QDialog
{
Q_OBJECT

public:
    explicit WalletInfoDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~WalletInfoDialog() override;

private:
    void openWalletDir();

    Ui::WalletInfoDialog *ui;
    AppContext *m_ctx;
};

#endif //WOWLET_WALLETINFODIALOG_H
