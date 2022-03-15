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
    void onStopped();
    void onClearClicked();
    void onBlockReward();
    void onRigDownloads(const QJsonObject &data);
    void onWownerodDownloads(const QJsonObject &data);
    void rigLinkClicked();
    void wownerodLinkClicked();
    void onProcessError(const QString &msg);
    void onProcessOutput(const QByteArray &msg);
    void onHashrate(const QString &hashrate);

private slots:
    void onBrowseClicked();
    void onThreadsValueChanged(int date);

signals:
    void miningStarted();
    void miningEnded();

private:
    void showContextRigMenu(const QPoint &pos);
    void showContextWownerodMenu(const QPoint &pos);
    void appendText(const QString &line);

    AppContext *m_ctx;
    Ui::XMRigWidget *ui;
    QStandardItemModel *m_modelRig;
    QStandardItemModel *m_modelWownerod;
    QMenu *m_contextMenuRig;
    QMenu *m_contextMenuWownerod;
    int m_threads;
    QStringList m_urlsRig;
    QStringList m_urlsWownerod;
    unsigned int m_consoleBuffer = 0;
    unsigned int m_consoleBufferMax = 2000;
};

#endif // REDDITWIDGET_H
