// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QStandardItemModel>
#include <QTableWidget>
#include <QDesktopServices>

#include "model/ForumModel.h"
#include "forumwidget.h"
#include "ui_forumwidget.h"
#include "utils/utils.h"
#include "utils/config.h"

ForumWidget::ForumWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ForumWidget),
    m_model(new ForumModel(this)),
    m_contextMenu(new QMenu(this))
{
    ui->setupUi(this);
    ui->tableView->setModel(m_model);
    this->setupTable();

    m_contextMenu->addAction("View thread", this, &ForumWidget::linkClicked);
    m_contextMenu->addAction("Copy link", this, &ForumWidget::copyUrl);
    connect(ui->tableView, &QHeaderView::customContextMenuRequested, this, &ForumWidget::showContextMenu);

    connect(ui->tableView, &QTableView::doubleClicked, this, &ForumWidget::linkClicked);

    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

ForumModel* ForumWidget::model() {
    return m_model;
}

void ForumWidget::linkClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    auto post = m_model->post(index.row());

    if (post)
        Utils::externalLinkWarning(this, post->permalink);
}

void ForumWidget::copyUrl() {
    QModelIndex index = ui->tableView->currentIndex();
    auto post = m_model->post(index.row());

    if (post) {
        Utils::copyToClipboard(post->permalink);
        emit setStatusText("Link copied to clipboard", true, 1000);
    }
}

void ForumWidget::setupTable() {
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch);
}

void ForumWidget::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->tableView->indexAt(pos);
        if (!index.isValid()) {
        return;
    }

    m_contextMenu->exec(ui->tableView->viewport()->mapToGlobal(pos));
}

ForumWidget::~ForumWidget() {
    delete ui;
}
