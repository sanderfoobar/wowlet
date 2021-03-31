// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_FPROCESS_H
#define WOWLET_FPROCESS_H

#include <QProcess>

#if defined(HAVE_SYS_PRCTL_H) && defined(Q_OS_UNIX)
#include <signal.h>
#include <sys/prctl.h>
#endif

class ChildProcess : public QProcess {
    Q_OBJECT
public:
    explicit ChildProcess(QObject* parent = nullptr);
    ~ChildProcess();
protected:
    void setupChildProcess() override;
};


#endif //WOWLET_FPROCESS_H
