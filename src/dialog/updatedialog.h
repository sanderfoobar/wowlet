// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_UPDATEDIALOG_H
#define WOWLET_UPDATEDIALOG_H

#include <QDialog>
#include "appcontext.h"

namespace Ui {
    class UpdateDialog;
}

class UpdateDialog : public QDialog
{
Q_OBJECT

public:
    explicit UpdateDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~UpdateDialog() override;

private slots:
    void checkboxIgnoreWarning(bool checked);

private:
    AppContext *ctx = nullptr;
    Ui::UpdateDialog *ui;
};


#endif //WOWLET_UPDATEDIALOG_H
