// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_TXPROOFWIDGET_H
#define WOWLET_TXPROOFWIDGET_H

#include <QWidget>

#include "libwalletqt/Wallet.h"
#include "libwalletqt/TransactionInfo.h"

namespace Ui {
    class TxProofWidget;
}

class TxProofWidget : public QWidget
{
Q_OBJECT

public:
    explicit TxProofWidget(QWidget *parent, Wallet *wallet, TransactionInfo *txid);
    ~TxProofWidget() override;

private:
    void copySpendProof();
    void copyTxProof();

    Ui::TxProofWidget *ui;
    QString m_txid;
    Wallet *m_wallet;
};

#endif //WOWLET_TXPROOFWIDGET_H
