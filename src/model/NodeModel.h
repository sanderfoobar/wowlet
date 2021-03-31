// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_NODEMODEL_H
#define WOWLET_NODEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

class WowletNode;

class NodeModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum ModelColumn {
        URL,
        Height,
        COUNT
    };

    explicit NodeModel(int nodeSource, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    WowletNode node(int row);

    void clear();
    void updateNodes(QList<WowletNode> nodes);

private:
    QList<WowletNode> m_nodes;
    QIcon m_offline;
    QIcon m_online;
    int m_nodeSource;
};

#endif //WOWLET_NODEMODEL_H
