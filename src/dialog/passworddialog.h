// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_PASSWORDDIALOG_H
#define WOWLET_PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
    class PasswordDialog;
}

class PasswordDialog : public QDialog
{
Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent, const QString &walletName, bool incorrectPassword);
    ~PasswordDialog() override;

    QString password = "";

private:
    Ui::PasswordDialog *ui;
};

#endif //WOWLET_PASSWORDDIALOG_H
