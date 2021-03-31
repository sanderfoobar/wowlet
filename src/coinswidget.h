// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_COINSWIDGET_H
#define WOWLET_COINSWIDGET_H

#include "appcontext.h"
#include "model/CoinsModel.h"
#include "model/CoinsProxyModel.h"
#include "libwalletqt/Coins.h"

#include <QWidget>
#include <QtSvg/QSvgWidget>

namespace Ui {
    class CoinsWidget;
}

class CoinsWidget : public QWidget
{
Q_OBJECT

public:
    explicit CoinsWidget(QWidget *parent = nullptr);
    void setModel(CoinsModel * model, Coins * coins);
    ~CoinsWidget() override;

public slots:
    void resetModel();

private slots:
    void showHeaderMenu(const QPoint& position);
    void setShowSpent(bool show);
    void freezeOutput();
    void freezeAllSelected();
    void thawOutput();
    void thawAllSelected();
    void viewOutput();
    void onSweepOutput();

signals:
    void freeze(QVector<int> indexes);
    void thaw(QVector<int> indexes);
    void sweepOutput(const QString &keyImage, const QString &address, bool isChurn, int outputs);

private:
    enum copyField {
        PubKey = 0,
        KeyImage,
        TxID,
        Address,
        Label,
        Height,
        Amount
    };

    Ui::CoinsWidget *ui;

    QMenu *m_contextMenu;
    QMenu *m_copyMenu;
    QAction *m_showSpentAction;
    QMenu *m_headerMenu;
    QAction *m_freezeOutputAction;
    QAction *m_freezeAllSelectedAction;
    QAction *m_thawOutputAction;
    QAction *m_thawAllSelectedAction;
    QAction *m_viewOutputAction;
    QAction *m_sweepOutputAction;
    Coins *m_coins;
    CoinsModel * m_model;
    CoinsProxyModel * m_proxyModel;
    AppContext *m_ctx;

    void showContextMenu(const QPoint & point);
    void copy(copyField field);
};


#endif //WOWLET_COINSWIDGET_H
