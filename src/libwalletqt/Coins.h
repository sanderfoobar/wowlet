// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_COINS_H
#define WOWLET_COINS_H

#include <functional>

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <QDateTime>
#include <wallet/api/wallet2_api.h>

namespace Monero {
    struct TransactionHistory;
}

class CoinsInfo;

class Coins : public QObject
{
Q_OBJECT
    Q_PROPERTY(int count READ count)

public:
    Q_INVOKABLE bool coin(int index, std::function<void (CoinsInfo &)> callback);
    Q_INVOKABLE void refresh(quint32 accountIndex);
    Q_INVOKABLE void refreshUnlocked();
    Q_INVOKABLE void freeze(int index) const;
    Q_INVOKABLE void thaw(int index) const;
    Q_INVOKABLE QStringList coins_from_txid(const QString &txid); // Todo: return CoinsInfo vector

    quint64 count() const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void coinFrozen() const;
    void coinThawed() const;

private:
    explicit Coins(Monero::Coins * pimpl, QObject *parent = nullptr);

private:
    friend class Wallet;
    mutable QReadWriteLock m_lock;
    Monero::Coins * m_pimpl;
    mutable QList<CoinsInfo*> m_tinfo;
};

#endif //WOWLET_COINS_H
