// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "tickerwidget.h"
#include "ui_tickerwidget.h"

#include "appcontext.h"
#include "utils/config.h"
#include "mainwindow.h"

TickerWidget::TickerWidget(QWidget *parent, QString symbol, QString title, bool convertBalance) :
    QWidget(parent),
    ui(new Ui::TickerWidget),
    m_symbol(std::move(symbol)),
    m_convertBalance(convertBalance)
{
    ui->setupUi(this);

    // default values before API data
    if (title == "") title = m_symbol;
    this->ui->tickerBox->setTitle(title);
    QString defaultPct = "0.0";
    QString defaultFiat = "...";

    this->setFontSizes();
    this->setPctText(defaultPct, true);
    this->setFiatText(defaultFiat, 0.0, true);

    connect(AppContext::prices, &Prices::fiatPricesUpdated, this, &TickerWidget::init);
    connect(AppContext::prices, &Prices::cryptoPricesUpdated, this, &TickerWidget::init);
    if (convertBalance)
        connect(MainWindow::getContext(), &AppContext::balanceUpdated, this, &TickerWidget::init);
}

void TickerWidget::init() {
    if(!AppContext::prices->markets.count() || !AppContext::prices->rates.count())
        return;

    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();

    if(!AppContext::prices->rates.contains(fiatCurrency)){
        config()->set(Config::preferredFiatCurrency, "USD");
        return;
    }

    double amount = m_convertBalance ? AppContext::balance : 1.0;
    double conversion = AppContext::prices->convert(m_symbol, fiatCurrency, amount);
    if (conversion < 0) return;
    setPercentHidden(conversion == 0);

    auto markets = AppContext::prices->markets;
    if(!markets.contains(m_symbol)) return;

    auto pct24h = markets[m_symbol].price_usd_change_pct_24h;
    auto pct24hText = QString::number(pct24h, 'f', 2);

    this->setPctText(pct24hText, pct24h >= 0.0);
    this->setFiatText(fiatCurrency, conversion, true);
}

void TickerWidget::setFiatText(QString &fiatCurrency, double amount, bool round) {
    QString number;
    if(round)
        number = QString::number(amount, 'f', 2);
    else
        number = QString::number(amount);

    auto conversionText = QString("%1 %2")\
            .arg(number)
            .arg(fiatCurrency);
    ui->tickerFiat->setText(conversionText);
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

void TickerWidget::removePctContainer() {
    ui->tickerPct->deleteLater();
}

void TickerWidget::setPercentHidden(bool hidden) {
    ui->tickerPct->setVisible(!hidden);
}

TickerWidget::~TickerWidget() {
    delete ui;
}