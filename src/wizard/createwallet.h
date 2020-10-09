// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_CREATEWALLET_H
#define FEATHER_CREATEWALLET_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"

namespace Ui {
    class CreateWalletPage;
}

class CreateWalletPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit CreateWalletPage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;

signals:
    void createWallet();

private:
    AppContext *m_ctx;
    QLabel *topLabel;
    Ui::CreateWalletPage *ui;
    QString m_walletDir;
    bool validateWidgets();
};

#endif //FEATHER_CREATEWALLET_H