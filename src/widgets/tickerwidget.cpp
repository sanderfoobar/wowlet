// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "tickerwidget.h"
#include "ui_tickerwidget.h"

#include "mainwindow.h"

TickerWidget::TickerWidget(QWidget *parent, QString title) :
    QWidget(parent),
    ui(new Ui::TickerWidget)
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

    QString defaultPct = "0.0";
    QString defaultFiat = "...";

    this->setFontSizes();
    this->setPctText(defaultPct, true);
    this->setFiatText(defaultFiat);
    this->setTitleText(title);

    connect(AppContext::prices, &Prices::fiatPricesUpdated, this, &TickerWidget::reload);
    connect(AppContext::prices, &Prices::cryptoPricesUpdated, this, &TickerWidget::reload);
    connect(m_ctx, &AppContext::balanceUpdated, this, &TickerWidget::reload);
}

void TickerWidget::setFiatText(QString &text) {
    ui->tickerFiat->setText(text);
}

void TickerWidget::setTitleText(QString text) {
    ui->tickerBox->setTitle(text);
}

void TickerWidget::hidePct(bool hide) {
    ui->tickerPct->setHidden(hide);
}

void TickerWidget::setPctText(QString &text, bool positive) {
    QString pctText = "<html><head/><body><p><span style=\" color:red;\">";
    if(positive) {
        pctText = pctText.replace("red", "green");
        pctText += QString("+%1%").arg(text);
    } else
        pctText += QString("%1%").arg(text);

    pctText += "</span></p></body></html>";
    ui->tickerPct->setText(pctText);
}

void TickerWidget::setFontSizes() {
    ui->tickerPct->setFont(Utils::relativeFont(-2));
    ui->tickerFiat->setFont(Utils::relativeFont(0));
}

TickerWidget::~TickerWidget() {
    delete ui;
}
