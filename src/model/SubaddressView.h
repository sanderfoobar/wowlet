// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_SUBADDRESSVIEW_H
#define WOWLET_SUBADDRESSVIEW_H

#include <QTreeView>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

class SubaddressView : public QTreeView
{

public:
    SubaddressView(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif //WOWLET_SUBADDRESSVIEW_H
