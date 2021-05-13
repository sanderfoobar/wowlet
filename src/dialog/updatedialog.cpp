// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QObject>
#include "updatedialog.h"
#include "ui_updatedialog.h"

#include "config-wowlet.h"

UpdateDialog::UpdateDialog(AppContext *ctx, QWidget *parent) :
        QDialog(parent),
        ctx(ctx),
        ui(new Ui::UpdateDialog) {
    ui->setupUi(this);
    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));

    auto version_str = ctx->versionPending.value("version").toString();

    QPixmap p(":assets/images/pls_update.jpg");
    ui->label_image->setPixmap(p.scaled(320, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->info_label->setText("Current version: " + QString(WOWLET_VERSION_SEMVER) + "\n"
                               "New version: " + version_str);

    ui->label_download->setText("Download: <a href=\"https://git.wownero.com/wowlet/wowlet/releases\">git.wownero.com</a>");
    ui->label_download->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->label_download->setOpenExternalLinks(true);

    // checkbox
    connect(ui->checkbox_ignore, &QCheckBox::clicked, this, &UpdateDialog::checkboxIgnoreWarning);

    this->adjustSize();
}

void UpdateDialog::checkboxIgnoreWarning(bool checked) {
    if(checked)
        config()->set(Config::ignoreUpdateWarning, WOWLET_VERSION_SEMVER);
    else
        config()->set(Config::ignoreUpdateWarning, "");
}

UpdateDialog::~UpdateDialog() {
    delete ui;
}
