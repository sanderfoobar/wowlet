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

    // tip amount slider
    ui->slider_tipAmount->setMinimum(1);
    ui->slider_tipAmount->setMaximum(60);
    ui->slider_tipAmount->setTickInterval(1);
    ui->slider_tipAmount->setSingleStep(1);
    double tipAmount = config()->get(Config::suchWowTipAmount).toDouble();
    ui->slider_tipAmount->setValue((int)(tipAmount * 10));
    connect(ui->slider_tipAmount, &QSlider::valueChanged, this, &SuchWowWidget::onTipSliderChanged);
    this->setTipAmountLabel();
}

void SuchWowWidget::setupTable() {
    ui->listWidget->setViewMode(QListWidget::IconMode);
    ui->listWidget->setIconSize(QSize(256, 256));
    ui->listWidget->setResizeMode(QListWidget::Adjust);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void SuchWowWidget::onWS(QJsonArray such_data) {
    if(this->m_ctx == nullptr)
        m_ctx = MainWindow::getContext();

    for (auto &&such_post: such_data) {
        auto obj = such_post.toObject();
        auto uid = obj.value("id").toInt();
        if(m_lookup.contains(uid)) continue;

        auto post = new SuchWowPost(m_ctx, this);
        post->added_by = obj.value("added_by").toString();
        post->addy = obj.value("addy").toString();
        post->title = obj.value("title").toString();
        post->img = obj.value("img").toString();
        post->thumb = obj.value("thumb").toString();
        post->href = obj.value("href").toString();;
        post->uid = uid;

        m_lookup[post->uid] = post;
        connect(post, &SuchWowPost::thumbReceived, this, &SuchWowWidget::addThumb);
        post->download_thumb();
    }
}

void SuchWowWidget::addThumb(SuchWowPost *post) {
    auto *item = new SuchWidgetItem(QIcon(post->thumb_data), post->title, post);
    ui->listWidget->addItem(item);
    ui->listWidget->sortItems();
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

    if(!post->img_data && !post->isFetchingImage()) {
        connect(post, &SuchWowPost::imgReceived, this, &SuchWowWidget::showImage);
        post->download_img();
        return;
    }

    this->showImage(post);
}

void SuchWowWidget::showImage(SuchWowPost *post) {
    const auto title = QString("%1 - %2").arg(post->title, post->added_by);
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

    for(const auto &such_key: m_lookup.keys()) {
        if(m_lookup[such_key]->title == title)
            return m_lookup[such_key];
    }

    return nullptr;
}

void SuchWowWidget::setTipAmountLabel(double tipAmount) {
    if(tipAmount == 0.0)
        tipAmount = config()->get(Config::suchWowTipAmount).toDouble();

    QString amount_fmt = Utils::amountToCurrencyString(tipAmount, config()->get(Config::preferredFiatCurrency).toString());
    ui->label_tipAmount->setText(QString("Default tip amount (%1)").arg(amount_fmt));
}

void SuchWowWidget::onTipSliderChanged(int value) {
    double amount = (double)value / 10;
    config()->set(Config::suchWowTipAmount, amount);
    setTipAmountLabel(amount);
}

void SuchWowWidget::onPreferredFiatCurrencyChanged(const QString &symbol) {
    this->setTipAmountLabel();
}

SuchWowWidget::~SuchWowWidget() {
    delete ui;
}
