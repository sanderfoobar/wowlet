// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef TICKERWIDGET_H
#define TICKERWIDGET_H

#include <QWidget>

#include "appcontext.h"

namespace Ui {
    class TickerWidget;
}

class TickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TickerWidget(QWidget *parent, QString title = "");
    void setFiatText(QString &text);
    void setTitleText(QString text);
    void setPctText(QString &text, bool positive);
    void setFontSizes();
    void hidePct(bool hide);
    ~TickerWidget() override;

signals:
    void reload();

private:
    Ui::TickerWidget *ui;
    AppContext *m_ctx;
};

#endif // TICKERWIDGET_H
