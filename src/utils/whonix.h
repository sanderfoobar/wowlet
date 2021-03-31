// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_WHONIX_H
#define WOWLET_WHONIX_H

#include <QString>

struct WhonixOS {
    static bool detect();
    static QString version();
};


#endif //WOWLET_WHONIX_H
