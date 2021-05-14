// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef SUCHWOWWIDGET_H
#define SUCHWOWWIDGET_H

#include <QObject>
#include <QMenu>
#include <QWidget>
#include <QItemSelection>
#include <QListWidgetItem>

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
    uint uid;

    QPixmap thumb_data;
    QPixmap img_data;

    void download_thumb() {
        const QString url = QString("%1/%2").arg(m_weburl, this->thumb);
        connect(m_networkThumb, &UtilsNetworking::webReceived, this, &SuchWowPost::onThumbReceived);
        m_networkThumb->get(url);
    }

    void download_img() {
        const QString url = QString("%1/%2").arg(m_weburl, this->img);
        connect(m_networkImg, &UtilsNetworking::webReceived, this, &SuchWowPost::onImgReceived);
        m_networkImg->get(url);
    }

    bool isFetchingImage() { return m_networkImg->busy; }
    bool isFetchingThumb() { return m_networkThumb->busy; }

private slots:
    void onThumbReceived(QByteArray data);
    void onImgReceived(QByteArray data);

signals:
    void imgReceived(SuchWowPost *post);
    void thumbReceived(SuchWowPost *post);

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
    void onPreferredFiatCurrencyChanged(const QString &symbol);

private slots:
    void addThumb(SuchWowPost *post);
    void showImage(SuchWowPost *post);
    void onTipSliderChanged(int value);

signals:
    void donate(QString donate);
    void openImage(SuchWowPost *post);

private:
    void setupTable();
    void suchDonate();
    void suchImage();
    void setTipAmountLabel(double tipAmount = 0.0);
    SuchWowPost* itemToPost();
    void showContextMenu(const QPoint &pos);

    QMap<uint, SuchWowPost*> m_lookup;
    Ui::SuchWowWidget *ui;
    AppContext *m_ctx = nullptr;
    QMenu *m_contextMenu;
};

class SuchWidgetItem : public QListWidgetItem
{
public:
    explicit SuchWidgetItem(const QIcon &icon, const QString &text, SuchWowPost *post, QListWidget *parent = nullptr, int type = Type) {
        this->setIcon(icon);
        this->setText(text);
        this->post = post;
    }

    SuchWowPost *post;

    // sort
    virtual bool operator< (const QListWidgetItem &other) const {
        auto const *_other = dynamic_cast<const SuchWidgetItem *>(&other);
        return this->post->uid > _other->post->uid;
    }
};

#endif // SUCHWOWWIDGET_H