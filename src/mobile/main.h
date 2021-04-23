// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_MAIN_H
#define WOWLET_MAIN_H

#include <QtCore>
#include <QQmlError>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QGuiApplication>
#include <QClipboard>
#include <QTimer>
#include <globals.h>

#include "appcontext.h"
#include "utils/config.h"
#include "QR-Code-scanner/Decoder.h"

namespace mobile {

    class Mobile : public QObject {
        Q_OBJECT
    public:
        explicit Mobile(AppContext *ctx, QCommandLineParser *cmdargs, QObject *parent = nullptr);
        ~Mobile() override;

        QList<QQmlError> errors;

        Q_INVOKABLE double cdiv(double amount) { return amount / globals::cdiv; }
        Q_INVOKABLE double add(double x, double y) const { return Utils::roundUp(x + y, 4); }  // round ceil 4 decimals
        Q_INVOKABLE double sub(double x, double y) const { return Utils::roundUp(x - y, 4); }  // round ceil 4 decimals

        Q_INVOKABLE void onCreateTransaction(const QString &address, const QString &amount_str, const QString description, bool all) {
            auto amount = WalletManager::amountFromString(amount_str);
            ctx->onCreateTransaction(address, amount, description, false);
        }

        Q_INVOKABLE void setClipboard(const QString &text) {
            m_pClipboard->setText(text, QClipboard::Clipboard);
            m_pClipboard->setText(text, QClipboard::Selection);
        }

        Q_INVOKABLE QString preferredFiat() {
            return config()->get(Config::preferredFiatCurrency).toString();
        }

        Q_INVOKABLE QString fiatToWow(double amount) {
            auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
            if (amount <= 0) return QString("0.00");

            double conversionAmount = AppContext::prices->convert(preferredFiatCurrency, "WOW", amount);
            return QString("%1").arg(QString::number(conversionAmount, 'f', 2));
        }

        Q_INVOKABLE QString wowToFiat(double amount) {
            auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
            if (amount <= 0) return QString("0.00");

            double conversionAmount = AppContext::prices->convert("WOW", preferredFiatCurrency, amount);
            if(conversionAmount <= 0) return QString("0.00");
            return QString("~%1").arg(QString::number(conversionAmount, 'f', 2));
        }

        Q_INVOKABLE void takeQRScreenshot();

        signals:
            void qrScreenshotFailed(QString error);
        void qrScreenshotSuccess(QString address);

    private slots:
        void onCheckQRScreenshot();

    private:
        AppContext *ctx;
        QQmlApplicationEngine m_engine;

        bool desktopMode = false;
        QString m_qrScreenshotPreviewPath;
        QString m_qrScreenshotImagePath;

        QCommandLineParser *m_parser;
        QClipboard *m_pClipboard;
        QTimer m_qrScreenshotTimer;
        QrDecoder m_qrDecoder;

        static QString checkQRScreenshotResults(std::vector<std::string> results);
    };

}

#endif //WOWLET_MAIN_H
