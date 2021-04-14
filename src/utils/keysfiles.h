// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef KEYSFILES_H
#define KEYSFILES_H

#include "libwalletqt/WalletManager.h"
#include "utils/networktype.h"
#include "utils/utils.h"

class WalletKeysFiles
{
    Q_GADGET
public:
    WalletKeysFiles();
    WalletKeysFiles(const QFileInfo &info, int networkType, QString address);

    QString fileName() const;
    qint64 modified() const;
    QString path() const;
    int networkType() const;
    QString address() const;

    QJsonObject toJsonObject() const;
    QVariant toVariant() const;

    Q_PROPERTY(qint64 modified READ modified)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString address READ address)
    Q_PROPERTY(int networkType READ networkType)

private:
    QString m_fileName;
    qint64 m_modified;
    QString m_path;
    int m_networkType;
    QString m_address;
};
Q_DECLARE_METATYPE(WalletKeysFiles)


class WalletKeysFilesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ModelColumns {
        NetworkType = 0,
        FileName,
        Path,
        Modified
    };

    explicit WalletKeysFilesModel(AppContext *ctx, QObject *parent = nullptr);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void clear();

    void findWallets();
    void addWalletKeysFile(const WalletKeysFiles &walletKeysFile);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QStringList walletDirectories;

    QList<WalletKeysFiles> listWallets() {
        return m_walletKeyFiles;
    }

private:
    void updateDirectories();

    AppContext *m_ctx;
    QList<WalletKeysFiles> m_walletKeyFiles;
    QAbstractItemModel *m_walletKeysFilesItemModel;
    QSortFilterProxyModel m_walletKeysFilesModelProxy;
};

class WalletKeysFilesProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit WalletKeysFilesProxyModel(QObject *parent, NetworkType::Type nettype);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    NetworkType::Type m_nettype;
};

#endif // KEYSFILES_H
