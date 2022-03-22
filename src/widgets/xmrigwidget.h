// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef XMRIGWIDGET_H
#define XMRIGWIDGET_H

#include <QObject>
#include <QQuickWidget>
#include <QQuickView>
#include <QQmlContext>
#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "utils/xmrig.h"
#include "utils/config.h"
#include "appcontext.h"
#include "globals.h"

namespace Ui {
    class XMRigWidget;
}

class XMRigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit XMRigWidget(AppContext *ctx, QWidget *parent = nullptr);
    ~XMRigWidget() override;

    Q_PROPERTY(int daemonMiningState READ daemonMiningState NOTIFY daemonMiningStateChanged);
    int daemonMiningState() const { return m_daemonMiningState; }

    QStandardItemModel *model();

public slots:
    void onWalletClosed();
    void onWalletOpened(Wallet *wallet);
    void onStartClicked();
    void onStopClicked();
    void onClearClicked();
    void onBlockReward();
    void onRigDownloads(const QJsonObject &data);
    void onWownerodDownloads(const QJsonObject &data);
    void rigLinkClicked();
    void wownerodLinkClicked();
    void onProcessError(const QString &msg);
    void onProcessOutput(const QByteArray &msg);
    void onHashrate(const QString &rate);
    void onDaemonStateChanged(DaemonMiningState state);
    void onSyncStatus(unsigned int from, unsigned int to, unsigned int pct);
    void onUptimeChanged(const QString &uptime);
    void onMenuTabChanged(int index);

private slots:
    void onBrowseClicked();
    void onThreadsValueChanged(int date);
    void onSimplifiedMiningChanged(int idx);

signals:
    void daemonOutput(const QString &line);
    void syncStatus(unsigned int from, unsigned int to, unsigned int pct);
    void hashrate(const QString &rate);
    void daemonMiningStateChanged();
    void uptimeChanged(const QString &uptime);

//protected:
//    void resizeEvent(QResizeEvent *event) override;

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
    unsigned int m_tabIndex = 0;
    unsigned int m_consoleBuffer = 0;
    unsigned int m_consoleBufferMax = 2000;
    int m_daemonMiningState = 0;

    QQuickWidget *m_quickWidget = nullptr;

    void resetUI();
    void startUI();

    void initConsole();
    void initQML();
    void destroyQml();
};

#endif
