// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_WIZARDVIEWONLY_H
#define WOWLET_WIZARDVIEWONLY_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>

#include "appcontext.h"

namespace Ui {
    class ViewOnlyPage;
}

class ViewOnlyPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ViewOnlyPage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;
    void cleanupPage() const;

private:
    AppContext *m_ctx;
    QLabel *topLabel;
    Ui::ViewOnlyPage *ui;
};

#endif
