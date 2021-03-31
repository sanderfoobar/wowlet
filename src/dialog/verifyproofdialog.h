// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_VERIFYPROOFDIALOG_H
#define WOWLET_VERIFYPROOFDIALOG_H

#include <QDialog>
#include "libwalletqt/Wallet.h"

namespace Ui {
    class VerifyProofDialog;
}

class VerifyProofDialog : public QDialog
{
Q_OBJECT

public:
    explicit VerifyProofDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~VerifyProofDialog() override;

private slots:
    void checkProof();

private:
    void checkTxProof(const QString &txId, const QString &address, const QString &message, const QString &signature);
    void checkSpendProof();
    void checkOutProof();
    void checkInProof();

    Ui::VerifyProofDialog *ui;
    Wallet *m_wallet;
};

#endif //WOWLET_VERIFYPROOFDIALOG_H
