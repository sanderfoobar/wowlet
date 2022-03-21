// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_XMRIG_H
#define WOWLET_XMRIG_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <QRegExp>
#include <QtNetwork>
#include <QApplication>
#include <QMainWindow>

#include "utils/childproc.h"

enum DaemonMiningState {
  idle = 0,
  startup,
  syncing,
  mining
};

class AppContext;
class XmRig : public QObject
{
Q_OBJECT

public:
    explicit XmRig(const QString &configDir, QObject *parent = nullptr);
    void prepare();

    bool start(const QString &path, int threads);
    void stop();

    DaemonMiningState daemonMiningState = DaemonMiningState::idle;

signals:
    void error(const QString &msg);
    void output(const QByteArray &data);
    void blockReward();
    void syncStatus(unsigned int from, unsigned int to, unsigned int pct);
    void hashrate(const QString &rate);
    void daemonStateChanged(DaemonMiningState state);
    void uptimeChanged(QString &uptime);

private slots:
    void onProcessStateChanged(QProcess::ProcessState);
    void onHandleProcessOutput();
    void onHandleProcessError(QProcess::ProcessError error);

private:
    void changeDaemonState(DaemonMiningState state);

    ChildProcess m_process;
    AppContext *m_ctx;
    QTimer *m_statusTimer;
    unsigned int m_to;
    unsigned int m_from;
};

#endif //WOWLET_XMRIG_H
