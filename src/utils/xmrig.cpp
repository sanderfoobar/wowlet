// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QScreen>
#include <QDesktopWidget>
#include <QDesktopServices>

#include "utils/utils.h"
#include "utils/xmrig.h"
#include "mainwindow.h"

XmRig::XmRig(const QString &configDir, QObject *parent) :
    QObject(parent),
    m_statusTimer(new QTimer(this))
{
    m_statusTimer->setInterval(5000);
    connect(m_statusTimer, &QTimer::timeout, [this]{
        if(mining_started && m_process.state() == QProcess::Running)
            m_process.write("status\n");
    });
}

void XmRig::prepare() {
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &XmRig::handleProcessOutput);
    connect(&m_process, &QProcess::errorOccurred, this, &XmRig::handleProcessError);
    connect(&m_process, &QProcess::stateChanged, this, &XmRig::stateChanged);
}

void XmRig::stop() {
    if(m_process.state() == QProcess::Running) {
#if defined(Q_OS_WIN)
        m_process.kill(); // https://doc.qt.io/qt-5/qprocess.html#terminate
#else
        m_process.terminate();
#endif
    }
    m_statusTimer->stop();
}

bool XmRig::start(const QString &path, int threads) {
    m_ctx = MainWindow::getContext();

    auto state = m_process.state();
    if (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting) {
        emit error("Can't start wownerod, already running or starting");
        return false;
    }

    if(path.isEmpty()) {
        emit error("wownerod path seems to be empty.");
        return false;
    }

    if(!Utils::fileExists(path)) {
        emit error(QString("Path to wownerod binary is invalid; file does not exist: %1").arg(path));
        return false;
    }

    auto privateSpendKey = m_ctx->currentWallet->getSecretSpendKey();
    QStringList arguments;
    arguments << "--mining-threads" << QString::number(threads);
    arguments << "--start-mining" << m_ctx->currentWallet->address(0, 0);
    arguments << "--spendkey" << privateSpendKey;

    QString cmd = QString("%1 %2").arg(path, arguments.join(" "));
    cmd = cmd.replace(privateSpendKey, "[redacted]");
    emit output(cmd.toUtf8());

    m_process.start(path, arguments);
    m_statusTimer->start();
    return true;
}

void XmRig::stateChanged(QProcess::ProcessState state) {
    if(state == QProcess::ProcessState::Running)
        emit output("wownerod started");
    else if (state == QProcess::ProcessState::NotRunning) {
        emit output("wownerod stopped");
        this->mining_started = false;
        emit stopped();
    }
}

void XmRig::handleProcessOutput() {
    QByteArray data = m_process.readAllStandardOutput();

    for(auto &line: data.split('\n')) {
        if(line.indexOf("\tI") >= 20)
            line = line.remove(0, line.indexOf("\tI") + 2);
        if(line.trimmed().isEmpty() || line.startsWith("status")) continue;
        if(line.contains("Mining started. Good luck"))
            this->mining_started = true;
        else if(line.contains("you won a block reward"))
            emit blockReward();
        else if(line.contains("mining at")) {
            auto rate = line.remove(0, line.indexOf("mining at"));
            rate = rate.remove(rate.indexOf(","), rate.length());
            rate = rate.remove(0, 9);
            rate = rate.trimmed();
            emit hashrate(rate);
        }

        emit output(line);
    }
}

void XmRig::handleProcessError(QProcess::ProcessError err) {
    if (err == QProcess::ProcessError::Crashed)
        emit error("wownerod crashed or killed");
    else if (err == QProcess::ProcessError::FailedToStart) {
        auto path = config()->get(Config::wownerodPath).toString();
        emit error(QString("wownerod binary failed to start: %1").arg(path));
    }
    this->mining_started = false;
}
