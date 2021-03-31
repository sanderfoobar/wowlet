// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_REDDITMODEL_H
#define WOWLET_REDDITMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "widgets/RedditPost.h"

class RedditModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        Title = 0,
        Author,
        Comments,
        COUNT
    };

    explicit RedditModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void updatePosts(const QList<QSharedPointer<RedditPost>>& posts);

    QSharedPointer<RedditPost> post(int row);

private:
    QList<QSharedPointer<RedditPost>> m_posts;
};

#endif //WOWLET_REDDITMODEL_H
