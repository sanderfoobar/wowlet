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

class AppContext;
class XmRig : public QObject
{
Q_OBJECT

public:
    explicit XmRig(const QString &configDir, QObject *parent = nullptr);
    void prepare();

    bool start(const QString &path, int threads);
    void stop();

signals:
    void error(const QString &msg);
    void output(const QByteArray &data);
    void stopped();
    void blockReward();
    void hashrate(const QString &rate);

private slots:
    void stateChanged(QProcess::ProcessState);
    void handleProcessOutput();
    void handleProcessError(QProcess::ProcessError error);

private:
    ChildProcess m_process;
    AppContext *m_ctx;
    QTimer *m_statusTimer;
    bool mining_started = false;
};

#endif //WOWLET_XMRIG_H
