// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FORUMWIDGET_H
#define FORUMWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "model/ForumModel.h"

namespace Ui {
    class ForumWidget;
}

class ForumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ForumWidget(QWidget *parent = nullptr);
    ~ForumWidget();
    ForumModel* model();

public slots:
    void linkClicked();

signals:
    void setStatusText(const QString &msg, bool override, int timeout);

private:
    void setupTable();
    void showContextMenu(const QPoint &pos);
    void copyUrl();

    Ui::ForumWidget *ui;
    ForumModel* const m_model;
    QMenu *m_contextMenu;
};

#endif // FORUMWIDGET_H
