// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_ADDRESSBOOKPROXYMODEL_H
#define WOWLET_ADDRESSBOOKPROXYMODEL_H

#include <QSortFilterProxyModel>

class AddressBookProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit AddressBookProxyModel(QObject* parent = nullptr);
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

public slots:
    void setSearchFilter(const QString& searchString){
        m_searchRegExp.setPattern(searchString);
        invalidateFilter();
    }

private:
    QRegExp m_searchRegExp;
};

#endif //WOWLET_ADDRESSBOOKPROXYMODEL_H
