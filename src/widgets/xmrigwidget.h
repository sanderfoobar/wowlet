// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef XMRIGWIDGET_H
#define XMRIGWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "utils/xmrig.h"
#include "utils/config.h"
#include "appcontext.h"

namespace Ui {
    class XMRigWidget;
}

class XMRigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit XMRigWidget(AppContext *ctx, QWidget *parent = nullptr);
    ~XMRigWidget() override;
    QStandardItemModel *model();

public slots:
    void onWalletClosed();
    void onWalletOpened(Wallet *wallet);
    void onStartClicked();
    void onStopClicked();
    void onClearClicked();
    void onDownloads(const QJsonObject &data);
    void linkClicked();
    void onProcessError(const QString &msg);
    void onProcessOutput(const QByteArray &msg);
    void onHashrate(const QString &hashrate);
    void onSoloChecked(int state);

private slots:
    void onBrowseClicked();
    void onThreadsValueChanged(int date);
    void onPoolChanged(int pos);

signals:
    void miningStarted();
    void miningEnded();

private:
    void showContextMenu(const QPoint &pos);

    AppContext *m_ctx;
    Ui::XMRigWidget *ui;
    QStandardItemModel *m_model;
    QMenu *m_contextMenu;
    int m_threads;
    QStringList m_urls;
    QStringList m_pools{"cryptonote.social:2223", "pool.hashvault.pro:8888"};
};

#endif // REDDITWIDGET_H
