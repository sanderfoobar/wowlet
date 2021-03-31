// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_CONTACTSDIALOG_H
#define WOWLET_CONTACTSDIALOG_H

#include <QDialog>

namespace Ui {
    class ContactsDialog;
}

class ContactsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContactsDialog(QWidget *parent = nullptr, const QString &address = "", const QString &name = "");
    ~ContactsDialog() override;

    QString getAddress();
    QString getName();

private:
    Ui::ContactsDialog *ui;

    QString m_address;
    QString m_name;
};

#endif //WOWLET_CONTACTSDIALOG_H
