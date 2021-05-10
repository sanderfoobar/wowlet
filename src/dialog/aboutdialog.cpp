// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "utils/utils.h"
#include "config-wowlet.h"

AboutDialog::AboutDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));

    QPixmap p(":assets/images/credits.jpg");
    ui->aboutImage->setPixmap(p.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    auto about = Utils::fileOpenQRC(":assets/about.txt");
    auto about_text = Utils::barrayToString(about);
    about_text = about_text.replace("<wowlet_version>", WOWLET_VERSION);
    about_text = about_text.replace("<wowlet_git_head>", WOWLET_BRANCH);
    about_text = about_text.replace("<current_year>", QString::number(QDate::currentDate().year()));
    ui->copyrightText->setPlainText(about_text);

    auto ack = Utils::fileOpenQRC(":assets/ack.txt");
    auto ack_text = Utils::barrayToString(ack);
    ui->ackText->setText(ack_text);

    m_model = new QStringListModel(this);

    QString contributors = Utils::barrayToString(Utils::fileOpenQRC(":assets/contributors.txt"));
    QStringList contributor_list = contributors.split("\n");
    m_model->setStringList(contributor_list);

    ui->authorView->setHeaderHidden(true);
    ui->authorView->setModel(this->m_model);
    ui->authorView->header()->setSectionResizeMode(QHeaderView::Stretch);

    this->adjustSize();
}

AboutDialog::~AboutDialog() {
    delete ui;
}

