// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_TXCONFADVDIALOG_H
#define WOWLET_TXCONFADVDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QAbstractButton>
#include <QMenu>
#include <QTextCharFormat>

#include "libwalletqt/PendingTransaction.h"
#include "appcontext.h"

namespace Ui {
    class TxConfAdvDialog;
}

class TxConfAdvDialog : public QDialog
{
Q_OBJECT

public:
    explicit TxConfAdvDialog(AppContext *ctx, const QString &description, QWidget *parent = nullptr);
    ~TxConfAdvDialog() override;

    void setTransaction(PendingTransaction *tx);
    void setUnsignedTransaction(UnsignedTransaction *utx);

private:
    void setupConstructionData(ConstructionInfo *ci);
    void signTransaction();
    void broadcastTransaction();
    void closeDialog();

    void unsignedCopy();
    void unsignedQrCode();
    void unsignedSaveFile();

    void signedCopy();
    void signedQrCode();
    void signedSaveFile();

    Ui::TxConfAdvDialog *ui;
    AppContext *m_ctx;
    PendingTransaction *m_tx = nullptr;
    UnsignedTransaction *m_utx = nullptr;
    QMenu *m_exportUnsignedMenu;
    QMenu *m_exportSignedMenu;
};

#endif //WOWLET_TXCONFADVDIALOG_H
