// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_WIZARDRESTORE_H
#define WOWLET_WIZARDRESTORE_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>

#include "utils/textedit.h"
#include "appcontext.h"

namespace Ui {
    class RestorePage;
}

class RestorePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit RestorePage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    void initializePage() override;
    int nextId() const override;

private:
    AppContext *m_ctx;
    Ui::RestorePage *ui;

    int m_mode = 14;
    QStringList m_words14;
    QStringList m_words25;
    QStringListModel *m_completer14Model = nullptr;
    QStringListModel *m_completer25Model = nullptr;
    QCompleter *m_completer14 = nullptr;
    QCompleter *m_completer25 = nullptr;
};

#endif
