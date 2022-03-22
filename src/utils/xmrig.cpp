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
        if(daemonMiningState == DaemonMiningState::mining && m_process.state() == QProcess::Running)
            m_process.write("status\n");
    });
}

void XmRig::prepare() {
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &XmRig::onHandleProcessOutput);
    connect(&m_process, &QProcess::errorOccurred, this, &XmRig::onHandleProcessError);
    connect(&m_process, &QProcess::stateChanged, this, &XmRig::onProcessStateChanged);
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
    if (state == QProcess::ProcessState::Running ||
        state == QProcess::ProcessState::Starting ||
        daemonMiningState != DaemonMiningState::idle) {
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

void XmRig::onProcessStateChanged(QProcess::ProcessState state) {
    if(state == QProcess::ProcessState::Running) {
        emit output("wownerod started");
        changeDaemonState(DaemonMiningState::startup);
    }
    else if (state == QProcess::ProcessState::NotRunning) {
        emit output("wownerod stopped");
        changeDaemonState(DaemonMiningState::idle);
    }
}

void XmRig::onHandleProcessOutput() {
    QByteArray data = m_process.readAllStandardOutput();

    for(auto &line: data.split('\n')) {
        // remove timestamp
        if(line.indexOf("\tI") >= 20)
            line = line.remove(0, line.indexOf("\tI") + 2);
        line = line.trimmed();

        // sad attempt at removing ANSI color codes
        // yes this is stupid, no i dont care
        // works remarkably well so far lmao
        auto ansi_start = QByteArray("\x1b\x5b");
        line = line.replace(ansi_start, "");
        line = line.replace("0;36m", "");
        line = line.replace("0;35m", "");
        line = line.replace("0;34m", "");
        line = line.replace("0;33m", "");
        line = line.replace("0;32m", "");
        line = line.replace("1;32m", "");
        line = line.replace("1;33m", "");
        line = line.replace("1;34m", "");
        line = line.replace("1;35m", "");
        line = line.replace("1;36m", "");
        if(line.startsWith("0m")) continue;

        auto lower = line.toLower();
        if(lower.isEmpty() || lower.startsWith("status")) continue;

        if(lower.startsWith("the daemon will start synchronizing")) {
            changeDaemonState(DaemonMiningState::startup);
        } else if(lower.startsWith("synchronization started")) {
            changeDaemonState(DaemonMiningState::syncing);
        } else if(lower.startsWith("synced") && lower.contains("left")) {
            if(daemonMiningState < DaemonMiningState::syncing) changeDaemonState(DaemonMiningState::syncing);
            QRegularExpression re("synced (\\d+)\\/(\\d+) \\((\\d+)%, (\\d+) left");

            QRegularExpressionMatch match = re.match(lower);
            if (match.hasMatch()) {
                auto from = match.captured(1);
                auto to = match.captured(2);
                auto pct = match.captured(3);
                m_from = from.toUInt();
                m_to = to.toUInt();
                emit syncStatus(m_from, m_to, pct.toInt());
            }
        } else if(lower.contains("mining started. good luck")) {
            emit syncStatus(m_to, m_to, 100);
            changeDaemonState(DaemonMiningState::mining);
        }
        else if(lower.contains("you won a block reward"))
            emit blockReward();
        else if(lower.contains("mining at")) {
            QRegularExpression re("Height\\: (\\d+)\\/(\\d+) \\((.*)\\) on mainnet, mining at (.*), net hash .*, uptime (.*)");

            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                m_from = match.captured(1).toUInt();
                m_to = match.captured(2).toUInt();
                unsigned int pct = match.captured(3).replace("%", "").toDouble();
                auto rate = match.captured(4);
                auto uptime = match.captured(5).replace(" ", "");

                emit uptimeChanged(uptime);
                emit syncStatus(m_to, m_to, pct);
                emit hashrate(rate);

                line = line.remove(0, line.indexOf("mining at"));
            }
        }

        emit output(line.trimmed());
    }
}

void XmRig::changeDaemonState(const DaemonMiningState state) {
    if(daemonMiningState == state) return;

    daemonMiningState = state;
    emit daemonStateChanged(daemonMiningState);
}

void XmRig::onHandleProcessError(QProcess::ProcessError err) {
    if (err == QProcess::ProcessError::Crashed)
        emit error("wownerod crashed or killed");
    else if (err == QProcess::ProcessError::FailedToStart) {
        auto path = config()->get(Config::wownerodPath).toString();
        emit error(QString("wownerod binary failed to start: %1").arg(path));
    }

    changeDaemonState(DaemonMiningState::idle);
}
