// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_FORUMMODEL_H
#define WOWLET_FORUMMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "widgets/ForumPost.h"

class ForumModel : public QAbstractTableModel
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

    explicit ForumModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void updatePosts(const QList<QSharedPointer<ForumPost>>& posts);

    QSharedPointer<ForumPost> post(int row);

private:
    QList<QSharedPointer<ForumPost>> m_posts;
};

#endif //WOWLET_FORUMMODEL_H
