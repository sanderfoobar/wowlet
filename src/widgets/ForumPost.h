// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_FORUMPOST_H
#define WOWLET_FORUMPOST_H

#include <QString>

struct ForumPost {
    ForumPost(const QString &title, const QString &author, const QString &permalink, int comments)
            : title(title), author(author), permalink(permalink), comments(comments){};

    QString title;
    QString author;
    QString permalink;
    int comments;
};

#endif //WOWLET_FORUMPOST_H
