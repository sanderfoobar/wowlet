// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_CSSPROGRESSDELEGATE_H
#define WOWLET_CSSPROGRESSDELEGATE_H

#include <QStyledItemDelegate>
#include "model/CCSModel.h"

class CCSProgressDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CCSProgressDelegate(CCSModel *model, QWidget *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    CCSModel *m_model;
};


#endif //WOWLET_CSSPROGRESSDELEGATE_H
