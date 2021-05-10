// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "ForumModel.h"

ForumModel::ForumModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

void ForumModel::clear() {
    beginResetModel();

    m_posts.clear();

    endResetModel();
}

void ForumModel::updatePosts(const QList<QSharedPointer<ForumPost>>& posts) {
    beginResetModel();

    m_posts.clear();
    for (const auto& post : posts) {
        m_posts.push_back(post);
    }

    endResetModel();
}

int ForumModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid()) {
        return 0;
    }
    return m_posts.count();
}

int ForumModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ModelColumn::COUNT;
}

QVariant ForumModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_posts.count())
        return QVariant();

    QSharedPointer<ForumPost> post = m_posts.at(index.row());

    if(role == Qt::DisplayRole) {
        switch(index.column()) {
            case Title:
                return post->title;
            case Author:
                return post->author;
            case Comments:
                return QString::number(post->comments);
            default:
                return QVariant();
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch(index.column()) {
            case Comments:
                return Qt::AlignRight;
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant ForumModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Title:
                return QString("Forum Post");
            case Author:
                return QString("Author");
            case Comments:
                return QString("Comments");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QSharedPointer<ForumPost> ForumModel::post(int row) {
    if (row < 0 || row >= m_posts.size()) {
        qCritical("%s: no forum post for index %d", __FUNCTION__, row);
        return QSharedPointer<ForumPost>();
    }

    return m_posts.at(row);
}