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
#include <globals.h>

#include "overlaycontroller.h"
#include "appcontext.h"
#include "utils/config.h"

namespace wowletvr {

    class WowletVR : public QObject {
        Q_OBJECT
    public:
        explicit WowletVR(AppContext *ctx, QCommandLineParser *cmdargs, QObject *parent = nullptr);
        ~WowletVR() override;

        void readThemes();
        void render();

        QList<QQmlError> errors;
        QVariantList theme_names;
        QVariantMap themes;

        Q_INVOKABLE QVariantMap getThemes() {
            return themes;
        }

        Q_INVOKABLE QString getCurrentTheme() {
            return config()->get(Config::openVRSkin).toString();
        }

        Q_INVOKABLE void setCurrentTheme(QString theme) {
            config()->set(Config::openVRSkin, theme);
        }

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

        Q_INVOKABLE QString amountToFiat(double amount) {
            auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
            if (amount <= 0) return QString("0.00 %1").arg(preferredFiatCurrency);

            double conversionAmount = AppContext::prices->convert("WOW", preferredFiatCurrency, amount);
            return QString("~%1 %2").arg(QString::number(conversionAmount, 'f', 2), preferredFiatCurrency);
        }

    private:
        AppContext *ctx;
        QCommandLineParser *m_parser;
        QQmlEngine m_engine;
        QQmlComponent *m_component;
        bool desktopMode = false;
        wowletvr::OverlayController *m_controller;
        QClipboard *m_pClipboard;
    };

}

#endif //WOWLET_MAIN_H
