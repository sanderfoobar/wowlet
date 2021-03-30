// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_WIZARDNETWORK_H
#define WOWLET_WIZARDNETWORK_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"
#include "utils/nodes.h"

namespace Ui {
    class NetworkPage;
}

class NetworkPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit NetworkPage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;

private:
    AppContext *m_ctx;
    QLabel *topLabel;
    Ui::NetworkPage *ui;
};

#endif //WOWLET_WIZARDNETWORK_H
