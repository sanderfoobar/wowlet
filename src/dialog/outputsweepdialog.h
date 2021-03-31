// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_OUTPUTSWEEPDIALOG_H
#define WOWLET_OUTPUTSWEEPDIALOG_H

#include <QDialog>

namespace Ui {
    class OutputSweepDialog;
}

class OutputSweepDialog : public QDialog
{
Q_OBJECT

public:
    explicit OutputSweepDialog(QWidget *parent = nullptr);
    ~OutputSweepDialog() override;

    QString address();
    bool churn() const;
    int outputs() const;

private:
    Ui::OutputSweepDialog *ui;

    QString m_address;
    bool m_churn;
    int m_outputs;
};


#endif //WOWLET_OUTPUTSWEEPDIALOG_H
