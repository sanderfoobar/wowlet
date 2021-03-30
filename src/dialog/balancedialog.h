// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_BALANCEDIALOG_H
#define WOWLET_BALANCEDIALOG_H

#include "libwalletqt/Wallet.h"

#include <QDialog>

namespace Ui {
    class BalanceDialog;
}

class BalanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BalanceDialog(QWidget *parent, Wallet *wallet);
    ~BalanceDialog() override;

private:
    Ui::BalanceDialog *ui;
};

#endif //WOWLET_BALANCEDIALOG_H
