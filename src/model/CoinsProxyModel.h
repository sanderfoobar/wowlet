// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_COINSPROXYMODEL_H
#define WOWLET_COINSPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "libwalletqt/Coins.h"

class CoinsProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit CoinsProxyModel(QObject* parent, Coins *coins);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

public slots:
    void setShowSpent(const bool showSpent){
        m_showSpent = showSpent;
        invalidateFilter();
    }

private:
    bool m_showSpent = false;
    Coins *m_coins;
};

#endif //WOWLET_COINSPROXYMODEL_H
