// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef WOWLET_INPUT_H
#define WOWLET_INPUT_H

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <utility>

class Input : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint64 amount READ amount)
    Q_PROPERTY(QString pubKey READ pubKey)

private:
    explicit Input(uint64_t _amount, QString _address,  QObject *parent = nullptr): QObject(parent), m_amount(_amount), m_pubkey(std::move(_address)) {};

    friend class ConstructionInfo;
    quint64 m_amount;
    QString m_pubkey;
public:
    quint64 amount() const { return m_amount; }
    QString pubKey() const { return m_pubkey; }

};

#endif //WOWLET_INPUT_H
