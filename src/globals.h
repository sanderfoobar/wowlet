// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_GLOBALS_H
#define WOWLET_GLOBALS_H

#include <QtGlobal>

namespace globals
{
    const qreal cdiv = 1e11;

    enum Tabs {
        HOME = 0,
        HISTORY,
        SEND,
        RECEIVE,
        COINS,
        CALC,
        XMRIG
    };

    enum TabsHome {
        FORUM,
        REDDIT,
        SUCHWOW,
        WFS
    };
}

#endif //WOWLET_GLOBALS_H
