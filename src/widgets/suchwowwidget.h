// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef SUCHWOWWIDGET_H
#define SUCHWOWWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "utils/networking.h"
#include "appcontext.h"

namespace Ui {
    class SuchWowWidget;
}


class SuchWowPost : public QObject
{
Q_OBJECT

public:
    explicit SuchWowPost(AppContext *ctx, QObject *parent = nullptr);
    ~SuchWowPost();

    QString title;
    QString added_by;
    QString thumb;
    QString img;
    QString addy;
    QString href;

    QPixmap thumb_data;
    QPixmap img_data;

    void download_thumb() {
        const QString url = QString("%1/%2").arg(m_weburl).arg(this->thumb);
        qDebug() << url;
        connect(m_networkThumb, &UtilsNetworking::webReceived, this, &SuchWowPost::onThumbReceived);
        m_networkThumb->get(url);
    }

    void download_img() {
        const QString url = QString("%1/%2").arg(m_weburl).arg(this->img);
        qDebug() << url;
        connect(m_networkImg, &UtilsNetworking::webReceived, this, &SuchWowPost::onImgReceived);
        m_networkImg->get(url);
    }

private slots:
    void onThumbReceived(QByteArray data);
    void onImgReceived(QByteArray data);

signals:
    void imgReceived(SuchWowPost *test);
    void thumbReceived(SuchWowPost *test);

private:
    QString m_weburl;
    AppContext *m_ctx = nullptr;
    UtilsNetworking *m_networkThumb = nullptr;
    UtilsNetworking *m_networkImg = nullptr;
};

class SuchWowWidget : public QWidget
{
Q_OBJECT

public:
    explicit SuchWowWidget(QWidget *parent = nullptr);
    ~SuchWowWidget();

public slots:
    void onWS(QJsonArray such_data);

private slots:
    void addThumb(SuchWowPost *test);

signals:
    void donate(QString donate);
    void openImage(SuchWowPost *test);

private:
    void setupTable();
    void suchDonate();
    void suchImage();
    SuchWowPost* itemToPost();
    void showContextMenu(const QPoint &pos);

    bool m_rendered = false;  // because we're too lazy to re-render on changes.
    Ui::SuchWowWidget *ui;
    QList<SuchWowPost*> m_posts;
    AppContext *m_ctx = nullptr;
    QMenu *m_contextMenu;
};

#endif // SUCHWOWWIDGET_H
