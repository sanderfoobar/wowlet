// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_MAIN_H
#define WOWLET_MAIN_H

#include <QtCore>
#include <QQmlError>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "overlaycontroller.h"
#include "appcontext.h"

namespace wowletvr {

    class WowletVR : public QObject {
        Q_OBJECT
    public:
        explicit WowletVR(AppContext *ctx, QCommandLineParser *cmdargs, QObject *parent = nullptr);
        ~WowletVR() override;

        void render();

        QList<QQmlError> errors;

    private:
        AppContext *ctx;
        QCommandLineParser *m_parser;
        QQmlEngine m_engine;
        QQmlComponent *m_component;
        bool desktopMode = false;
        wowletvr::OverlayController *m_controller;
    };

}

#endif //WOWLET_MAIN_H
