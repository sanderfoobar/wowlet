// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_CREATEWALLET_H
#define WOWLET_CREATEWALLET_H

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
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void createWallet();
    void defaultWalletDirChanged(QString walletDir);

private:
    AppContext *m_ctx;
    Ui::CreateWalletPage *ui;
    QString m_walletDir;
    bool validateWidgets();
};

#endif //WOWLET_CREATEWALLET_H
