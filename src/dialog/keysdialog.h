// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_KEYSDIALOG_H
#define WOWLET_KEYSDIALOG_H

#include <QDialog>
#include "appcontext.h"

namespace Ui {
    class KeysDialog;
}

class KeysDialog : public QDialog
{
Q_OBJECT

public:
    explicit KeysDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~KeysDialog() override;

private:
    Ui::KeysDialog *ui;
};


#endif //WOWLET_KEYSDIALOG_H
