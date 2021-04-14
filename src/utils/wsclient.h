// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QTimer>

class AppContext;
class WSClient : public QObject
{
    Q_OBJECT

public:
    explicit WSClient(AppContext *ctx, const QString &url, QObject *parent = nullptr);
    void start();
    void sendMsg(const QByteArray &data);
    QWebSocket webSocket;
    QString url;

signals:
    void closed();
    void connectionEstablished();
    void WSMessage(QJsonObject message);

private slots:
    void onConnected();
    void onbinaryMessageReceived(const QByteArray &message);
    void checkConnection();
    void onError(QAbstractSocket::SocketError error);

private:
    QTimer m_connectionTimer;
    QTimer m_pingTimer;
    AppContext *m_ctx;
    bool m_tor = true;
};

#endif // ECHOCLIENT_H