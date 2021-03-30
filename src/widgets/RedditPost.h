// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_REDDITPOST_H
#define WOWLET_REDDITPOST_H

#include <QString>

struct RedditPost {
    RedditPost(const QString &title, const QString &author, const QString &permalink, int comments)
            : title(title), author(author), permalink(permalink), comments(comments){};

    QString title;
    QString author;
    QString permalink;
    int comments;
};

#endif //WOWLET_REDDITPOST_H
