// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_TORINFODIALOG_H
#define WOWLET_TORINFODIALOG_H

#include <QDialog>

#include "appcontext.h"

namespace Ui {
    class TorInfoDialog;
}

class TorInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TorInfoDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~TorInfoDialog() override;

public slots:
    void onLogsUpdated();

private:
    Ui::TorInfoDialog *ui;
    AppContext *m_ctx;
};


#endif //WOWLET_TORINFODIALOG_H
