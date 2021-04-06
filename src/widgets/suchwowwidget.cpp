// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QStandardItemModel>
#include <QMessageBox>
#include <QTableWidget>
#include <QDesktopServices>
#include <QListWidgetItem>

#include "suchwowwidget.h"
#include "ui_suchwowwidget.h"
#include "utils/utils.h"
#include "utils/config.h"
#include "mainwindow.h"


SuchWowPost::SuchWowPost(AppContext *ctx, QObject *parent) :
        QObject(parent),
        m_ctx(ctx) {
    m_networkImg = new UtilsNetworking(m_ctx->network, this);
    m_networkThumb = new UtilsNetworking(m_ctx->network, this);
    m_weburl = QString("http://%1/suchwow").arg(this->m_ctx->wsUrl);
}

void SuchWowPost::onThumbReceived(QByteArray data) {
    thumb_data.loadFromData(data, "JPEG");
    emit thumbReceived(this);
}

void SuchWowPost::onImgReceived(QByteArray data) {
    img_data.loadFromData(data);
    emit imgReceived(this);
}

SuchWowPost::~SuchWowPost() = default;


SuchWowWidget::SuchWowWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SuchWowWidget),
        m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    this->setupTable();

    m_contextMenu->addAction("View image", this, &SuchWowWidget::suchImage);
    m_contextMenu->addAction("Donate", this, &SuchWowWidget::suchDonate);

    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &SuchWowWidget::showContextMenu);
}

void SuchWowWidget::setupTable() {
    ui->listWidget->setViewMode(QListWidget::IconMode);
    ui->listWidget->setIconSize(QSize(256, 256));
    ui->listWidget->setResizeMode(QListWidget::Adjust);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void SuchWowWidget::onWS(QJsonArray such_data) {
    if(this->m_rendered) return; // only draw once
    if(this->m_ctx == nullptr) {
        m_ctx = MainWindow::getContext();
    }

    for (auto &&such_post: such_data) {
        auto obj = such_post.toObject();
        auto s = new SuchWowPost(m_ctx, this);
        s->added_by = obj.value("added_by").toString();
        s->addy = obj.value("addy").toString();
        s->title = obj.value("title").toString();
        s->img = obj.value("img").toString();
        s->thumb = obj.value("thumb").toString();
        s->href = obj.value("href").toString();
        m_posts.push_back(s);

        connect(s, &SuchWowPost::thumbReceived, this, &SuchWowWidget::addThumb);
        s->download_thumb();
        s->download_img();
    }
}

void SuchWowWidget::addThumb(SuchWowPost *test) {
    auto *item = new QListWidgetItem(QIcon(test->thumb_data), test->title);
    ui->listWidget->addItem(item);
}

void SuchWowWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->listWidget->indexAt(pos);
    if (!index.isValid())
        return;
    m_contextMenu->exec(ui->listWidget->viewport()->mapToGlobal(pos));
}

void SuchWowWidget::suchImage() {
    auto *post = this->itemToPost();
    if(post == nullptr)
        return;
    if(!post->img_data) {
        QMessageBox::warning(this, "wh00pz", "Image not dowloaded yet.");
        return;
    }

    const auto title = QString("%1 - %2").arg(post->title).arg(post->added_by);
    QMessageBox mb(title, "",
                   QMessageBox::NoIcon,
                   QMessageBox::Ok,
                   QMessageBox::NoButton,
                   QMessageBox::NoButton);
    mb.setIconPixmap(post->img_data);
    mb.exec();
}

void SuchWowWidget::suchDonate() {
    auto *post = this->itemToPost();
    if(post != nullptr)
        emit donate(post->addy);
}

SuchWowPost *SuchWowWidget::itemToPost() {
    QListWidgetItem *item = ui->listWidget->currentItem();
    QString title = item->text();
    for(auto &post: m_posts){
        if(post->title == title) {
            return post;
        }
    }
    return nullptr;
}

SuchWowWidget::~SuchWowWidget() {
    delete ui;
}
