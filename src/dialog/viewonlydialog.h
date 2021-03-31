// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_VIEWONLYDIALOG_H
#define WOWLET_VIEWONLYDIALOG_H

#include <QDialog>
#include "appcontext.h"

namespace Ui {
    class ViewOnlyDialog;
}

class ViewOnlyDialog : public QDialog
{
Q_OBJECT

public:
    explicit ViewOnlyDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~ViewOnlyDialog() override;

private slots:
    void onWriteViewOnlyWallet();

private:
    Ui::ViewOnlyDialog *ui;
    AppContext *m_ctx = nullptr;
    void copyToClipboad();
};


#endif //WOWLET_KEYSDIALOG_H
