// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WOWLET_UTILS_H
#define WOWLET_UTILS_H

#include <QRegExp>
#include <QStandardItemModel>
#include <QApplication>
#include <QTextCharFormat>
#ifdef __ANDROID__
#include <QtAndroid>
#endif

#include <monero_seed/monero_seed.hpp>

#include "networktype.h"
#include "libwalletqt/Wallet.h"

struct logMessage
{
    logMessage(const QtMsgType &type, const QString &message, const QString &fn){
        logMessage::type = type;
        logMessage::message = message;
        logMessage::fn = fn;
    }
    QtMsgType type;
    QString message;
    QString fn;
};

struct xdgDesktopEntryPaths {
    QString pathApp;
    QString pathIcon;
    QString PathMime;
};

const xdgDesktopEntryPaths xdgPaths = {
    QString("%1/org.wowlet.wowlet.desktop").arg(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)),
    QString("%1/usr/share/icons/hicolor/64x64/apps/wowlet.png").arg(QDir::homePath()),
    QString("/")
};

class Utils
{

public:
    static bool portOpen(const QString &hostname, quint16 port);
    static bool fileExists(const QString &path);
    static QByteArray fileOpen(const QString &path);
    static QByteArray fileOpenQRC(const QString &path);
    static qint64 fileModifiedAge(const QString &path);
    static void desktopNotify(const QString &title, const QString &message, int duration);
    static bool fileWrite(const QString &path, const QString &data);
    static QStringList fileFind(const QRegExp &pattern, const QString &baseDir, int level, int depth, int maxPerDir);
    static bool validateJSON(const QByteArray &blob);
    static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);
    static void applicationLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void externalLinkWarning(QWidget *parent, const QString &url);
    static bool dirExists(const QString &path);
    static QString barrayToString(const QByteArray &data);
    static QStandardItem *qStandardItem(const QString &text);
    static QStandardItem *qStandardItem(const QString &text, QFont &font);
    static void copyToClipboard(const QString &string);
    static QString copyFromClipboard();
    static QString blockExplorerLink(const QString &txid);
    static QString getUnixAccountName();
    static QString xdgDesktopEntry();
    static bool xdgDesktopEntryWrite(const QString &path);
    static void xdgRefreshApplications();
    static bool xdgDesktopEntryRegister();
    static bool pixmapWrite(const QString &path, const QPixmap &pixmap);
    static QFont relativeFont(int delta);
    static double roundSignificant(double N, double n);
    static QString formatBytes(quint64 bytes);
    static QLocale getCurrencyLocale(const QString &currencyCode);
    static QString amountToCurrencyString(double amount, const QString &currencyCode);
    static int maxLength(const QVector<QString> &array);
    static double roundUp(double value, int decimal_places);
    static QMap<QString, QLocale> localeCache;
    static QString balanceFormat(quint64 balance);
    static QTextCharFormat addressTextFormat(const SubaddressIndex &index);
    static bool versionOutdated(const QString &current_version, const QString &newest_version);

    template<typename QEnum>
    static QString QtEnumToString (const QEnum value) {
        return QString::fromStdString(std::string(QMetaEnum::fromType<QEnum>().valueToKey(value)));
    }

#ifdef __ANDROID__
    static bool androidAskPermissions(const QVector<QString> &permissions);
#endif
};

class AppContext;  // forward declaration

#endif //WOWLET_UTILS_H
