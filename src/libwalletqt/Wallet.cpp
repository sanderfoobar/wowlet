// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.

#include "Wallet.h"
#include "PendingTransaction.h"
#include "UnsignedTransaction.h"
#include "TransactionHistory.h"
#include "AddressBook.h"
#include "Subaddress.h"
#include "SubaddressAccount.h"
#include "Coins.h"
#include "model/TransactionHistoryModel.h"
#include "model/TransactionHistoryProxyModel.h"
#include "model/AddressBookModel.h"
#include "model/SubaddressModel.h"
#include "model/SubaddressAccountModel.h"
#include "model/CoinsModel.h"
#include "wallet/api/wallet2_api.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QUrl>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QList>
#include <QVector>
#include <QMutexLocker>

namespace {
    static const int DAEMON_BLOCKCHAIN_HEIGHT_CACHE_TTL_SECONDS = 5;
    static const int DAEMON_BLOCKCHAIN_TARGET_HEIGHT_CACHE_TTL_SECONDS = 30;
    static const int WALLET_CONNECTION_STATUS_CACHE_TTL_SECONDS = 5;

    static constexpr char ATTRIBUTE_SUBADDRESS_ACCOUNT[] ="gui.subaddress_account";
}

Wallet::Wallet(QObject * parent)
    : Wallet(nullptr, parent)
{
}

QString Wallet::getSeed() const
{
    return QString::fromStdString(m_walletImpl->seed());
}

QString Wallet::getSeedLanguage() const
{
    return QString::fromStdString(m_walletImpl->getSeedLanguage());
}

void Wallet::setSeedLanguage(const QString &lang)
{
    m_walletImpl->setSeedLanguage(lang.toStdString());
}

Wallet::Status Wallet::status() const
{
    return static_cast<Status>(m_walletImpl->status());
}

NetworkType::Type Wallet::nettype() const
{
    return static_cast<NetworkType::Type>(m_walletImpl->nettype());
}


void Wallet::updateConnectionStatusAsync()
{
    m_scheduler.run([this] {
        if (m_connectionStatus == Wallet::ConnectionStatus_Disconnected)
        {
            setConnectionStatus(ConnectionStatus_Connecting);
        }
        ConnectionStatus newStatus = static_cast<ConnectionStatus>(m_walletImpl->connected());
        if (newStatus != m_connectionStatus || !m_initialized) {
            m_initialized = true;
            setConnectionStatus(newStatus);
        }
        // Release lock
        m_connectionStatusRunning = false;
    });
}

Wallet::ConnectionStatus Wallet::connected(bool forceCheck)
{
    // cache connection status
    if (forceCheck || !m_initialized || (m_connectionStatusTime.elapsed() / 1000 > m_connectionStatusTtl && !m_connectionStatusRunning) || m_connectionStatusTime.elapsed() > 30000) {
        m_connectionStatusRunning = true;
        m_connectionStatusTime.restart();
        updateConnectionStatusAsync();
    }

    return m_connectionStatus;
}

bool Wallet::disconnected() const
{
    return m_disconnected;
}

void Wallet::setConnectionStatus(ConnectionStatus value)
{
    if (m_connectionStatus == value)
    {
        return;
    }

    m_connectionStatus = value;
    emit connectionStatusChanged(m_connectionStatus);

    bool disconnected = m_connectionStatus == Wallet::ConnectionStatus_Connecting ||
        m_connectionStatus == Wallet::ConnectionStatus_Disconnected;

    if (m_disconnected != disconnected)
    {
        m_disconnected = disconnected;
        emit disconnectedChanged();
    }
}

bool Wallet::synchronized() const
{
    return m_walletImpl->synchronized();
}

QString Wallet::errorString() const
{
    return QString::fromStdString(m_walletImpl->errorString());
}

bool Wallet::setPassword(const QString &password)
{
    return m_walletImpl->setPassword(password.toStdString());
}

QString Wallet::address(quint32 accountIndex, quint32 addressIndex) const
{
    return QString::fromStdString(m_walletImpl->address(accountIndex, addressIndex));
}

QString Wallet::path() const
{
    return QDir::toNativeSeparators(QString::fromStdString(m_walletImpl->path()));
}



//void Wallet::storeAsync(const QVariant &callback, const QString &path /* = "" */)
//{
//    const auto future = m_scheduler.run(
//        [this, path] {
//            QMutexLocker locker(&m_storeMutex);

//            return QVariantList({m_walletImpl->store(path.toStdString())});
//        },
//        callback);
//    if (!future.first)
//    {
//        QVariant(callback).call(QVariantList({false}));
//    }
//}

void Wallet::store(const QString &path)
{
    m_walletImpl->store(path.toStdString());
}

bool Wallet::init(const QString &daemonAddress, bool trustedDaemon, quint64 upperTransactionLimit, bool isRecovering, bool isRecoveringFromDevice, quint64 restoreHeight)
{
    qDebug() << "init non async";
    if (isRecovering){
        qDebug() << "RESTORING";
        m_walletImpl->setRecoveringFromSeed(true);
    }
    if (isRecoveringFromDevice){
        qDebug() << "RESTORING FROM DEVICE";
        m_walletImpl->setRecoveringFromDevice(true);
    }
    if (isRecovering || isRecoveringFromDevice) {
        m_walletImpl->setRefreshFromBlockHeight(restoreHeight);
    }
    m_walletImpl->init(daemonAddress.toStdString(), upperTransactionLimit, m_daemonUsername.toStdString(), m_daemonPassword.toStdString());
    setTrustedDaemon(trustedDaemon);
    return true;
}

void Wallet::setDaemonLogin(const QString &daemonUsername, const QString &daemonPassword)
{
    // store daemon login
    m_daemonUsername = daemonUsername;
    m_daemonPassword = daemonPassword;
}

void Wallet::initAsync(const QString &daemonAddress, bool trustedDaemon, quint64 upperTransactionLimit, bool isRecovering, bool isRecoveringFromDevice, quint64 restoreHeight)
{
    qDebug() << "initAsync: " + daemonAddress;
    const auto future = m_scheduler.run([this, daemonAddress, trustedDaemon, upperTransactionLimit, isRecovering, isRecoveringFromDevice, restoreHeight] {
        bool success = init(daemonAddress, trustedDaemon, upperTransactionLimit, isRecovering, isRecoveringFromDevice, restoreHeight);
        if (success)
        {
            emit walletCreationHeightChanged();
            qDebug() << "init async finished - starting refresh";
            connected(true);
            m_walletImpl->startRefresh();
        }
    });
    if (future.first)
    {
        setConnectionStatus(Wallet::ConnectionStatus_Connecting);
    }
}

bool Wallet::isHwBacked() const
{
    return m_walletImpl->getDeviceType() != Monero::Wallet::Device_Software;
}

bool Wallet::isLedger() const
{
    return m_walletImpl->getDeviceType() == Monero::Wallet::Device_Ledger;
}

bool Wallet::isTrezor() const
{
    return m_walletImpl->getDeviceType() == Monero::Wallet::Device_Trezor;
}

//! create a view only wallet
bool Wallet::createViewOnly(const QString &path, const QString &password) const
{
    // Create path
    QDir d = QFileInfo(path).absoluteDir();
    d.mkpath(d.absolutePath());
    return m_walletImpl->createWatchOnly(path.toStdString(),password.toStdString(),m_walletImpl->getSeedLanguage());
}

bool Wallet::connectToDaemon()
{
    return m_walletImpl->connectToDaemon();
}

void Wallet::setTrustedDaemon(bool arg)
{
    m_walletImpl->setTrustedDaemon(arg);
}

bool Wallet::viewOnly() const
{
    return m_walletImpl->watchOnly();
}

quint64 Wallet::balance() const
{
    return balance(m_currentSubaddressAccount);
}

quint64 Wallet::balance(quint32 accountIndex) const
{
    return m_walletImpl->balance(accountIndex);
}

quint64 Wallet::balanceAll() const
{
    return m_walletImpl->balanceAll();
}

quint64 Wallet::unlockedBalance() const
{
    return unlockedBalance(m_currentSubaddressAccount);
}

quint64 Wallet::unlockedBalance(quint32 accountIndex) const
{
    return m_walletImpl->unlockedBalance(accountIndex);
}

quint64 Wallet::unlockedBalanceAll() const
{
    return m_walletImpl->unlockedBalanceAll();
}

quint32 Wallet::currentSubaddressAccount() const
{
    return m_currentSubaddressAccount;
}
void Wallet::switchSubaddressAccount(quint32 accountIndex)
{
    if (accountIndex < numSubaddressAccounts())
    {
        m_currentSubaddressAccount = accountIndex;
        if (!setCacheAttribute(ATTRIBUTE_SUBADDRESS_ACCOUNT, QString::number(m_currentSubaddressAccount)))
        {
            qWarning() << "failed to set " << ATTRIBUTE_SUBADDRESS_ACCOUNT << " cache attribute";
        }
        m_subaddress->refresh(m_currentSubaddressAccount);
        m_history->refresh(m_currentSubaddressAccount);
        emit currentSubaddressAccountChanged();
    }
}
void Wallet::addSubaddressAccount(const QString& label)
{
    m_walletImpl->addSubaddressAccount(label.toStdString());
    switchSubaddressAccount(numSubaddressAccounts() - 1);
}
quint32 Wallet::numSubaddressAccounts() const
{
    return m_walletImpl->numSubaddressAccounts();
}
quint32 Wallet::numSubaddresses(quint32 accountIndex) const
{
    return m_walletImpl->numSubaddresses(accountIndex);
}
void Wallet::addSubaddress(const QString& label)
{
    m_walletImpl->addSubaddress(currentSubaddressAccount(), label.toStdString());
}
QString Wallet::getSubaddressLabel(quint32 accountIndex, quint32 addressIndex) const
{
    return QString::fromStdString(m_walletImpl->getSubaddressLabel(accountIndex, addressIndex));
}
void Wallet::setSubaddressLabel(quint32 accountIndex, quint32 addressIndex, const QString &label)
{
    m_walletImpl->setSubaddressLabel(accountIndex, addressIndex, label.toStdString());
    emit currentSubaddressAccountChanged();
}
void Wallet::deviceShowAddressAsync(quint32 accountIndex, quint32 addressIndex, const QString &paymentId)
{
    m_scheduler.run([this, accountIndex, addressIndex, paymentId] {
        m_walletImpl->deviceShowAddress(accountIndex, addressIndex, paymentId.toStdString());
        emit deviceShowAddressShowed();
    });
}

void Wallet::refreshHeightAsync()
{
    m_scheduler.run([this] {
        quint64 daemonHeight;
        QPair<bool, QFuture<void>> daemonHeightFuture = m_scheduler.run([this, &daemonHeight] {
            daemonHeight = daemonBlockChainHeight();
        });
        if (!daemonHeightFuture.first)
        {
            return;
        }

        quint64 targetHeight;
        QPair<bool, QFuture<void>> targetHeightFuture = m_scheduler.run([this, &targetHeight] {
            targetHeight = daemonBlockChainTargetHeight();
        });
        if (!targetHeightFuture.first)
        {
            return;
        }

        quint64 walletHeight = blockChainHeight();
        daemonHeightFuture.second.waitForFinished();
        targetHeightFuture.second.waitForFinished();

        emit heightRefreshed(walletHeight, daemonHeight, targetHeight);
    });
}

quint64 Wallet::blockChainHeight() const
{
    return m_walletImpl->blockChainHeight();
}

quint64 Wallet::daemonBlockChainHeight() const
{
    // cache daemon blockchain height for some time (60 seconds by default)

    if (m_daemonBlockChainHeight == 0
            || m_daemonBlockChainHeightTime.elapsed() / 1000 > m_daemonBlockChainHeightTtl) {
        m_daemonBlockChainHeight = m_walletImpl->daemonBlockChainHeight();
        m_daemonBlockChainHeightTime.restart();
    }
    return m_daemonBlockChainHeight;
}

quint64 Wallet::daemonBlockChainTargetHeight() const
{
    if (m_daemonBlockChainTargetHeight <= 1
            || m_daemonBlockChainTargetHeightTime.elapsed() / 1000 > m_daemonBlockChainTargetHeightTtl) {
        m_daemonBlockChainTargetHeight = m_walletImpl->daemonBlockChainTargetHeight();

        // Target height is set to 0 if daemon is synced.
        // Use current height from daemon when target height < current height
        if (m_daemonBlockChainTargetHeight < m_daemonBlockChainHeight){
            m_daemonBlockChainTargetHeight = m_daemonBlockChainHeight;
        }
        m_daemonBlockChainTargetHeightTime.restart();
    }

    return m_daemonBlockChainTargetHeight;
}

bool Wallet::exportKeyImages(const QString& path)
{
    return m_walletImpl->exportKeyImages(path.toStdString());
}

bool Wallet::importKeyImages(const QString& path)
{
    return m_walletImpl->importKeyImages(path.toStdString());
}

bool Wallet::refresh()
{
    bool result = m_walletImpl->refresh();
    m_history->refresh(currentSubaddressAccount());
    m_subaddress->refresh(currentSubaddressAccount());
    m_coins->refresh(currentSubaddressAccount());
    m_subaddressAccount->getAll();
    if (result)
        emit updated();
    return result;
}

void Wallet::refreshAsync()
{
    qDebug() << "refresh async";
    m_walletImpl->refreshAsync();
}

void Wallet::setAutoRefreshInterval(int seconds)
{
    m_walletImpl->setAutoRefreshInterval(seconds);
}

int Wallet::autoRefreshInterval() const
{
    return m_walletImpl->autoRefreshInterval();
}

void Wallet::startRefresh() const
{
    m_walletImpl->startRefresh();
}

void Wallet::pauseRefresh() const
{
    m_walletImpl->pauseRefresh();
}

PendingTransaction *Wallet::createTransaction(const QString &dst_addr, const QString &payment_id,
                                              quint64 amount, quint32 mixin_count,
                                              PendingTransaction::Priority priority)
{
    std::set<uint32_t> subaddr_indices;
    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransaction(
                dst_addr.toStdString(), payment_id.toStdString(), amount, mixin_count,
                static_cast<Monero::PendingTransaction::Priority>(priority), currentSubaddressAccount(), subaddr_indices);
    PendingTransaction * result = new PendingTransaction(ptImpl,0);
    return result;
}

void Wallet::createTransactionAsync(const QString &dst_addr, const QString &payment_id,
                               quint64 amount, quint32 mixin_count,
                               PendingTransaction::Priority priority)
{
    m_scheduler.run([this, dst_addr, payment_id, amount, mixin_count, priority] {
        PendingTransaction *tx = createTransaction(dst_addr, payment_id, amount, mixin_count, priority);
        emit transactionCreated(tx, dst_addr, payment_id, mixin_count);
    });
}

PendingTransaction *Wallet::createTransactionAll(const QString &dst_addr, const QString &payment_id,
                                                 quint32 mixin_count, PendingTransaction::Priority priority)
{
    std::set<uint32_t> subaddr_indices;
    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransaction(
                dst_addr.toStdString(), payment_id.toStdString(), Monero::optional<uint64_t>(), mixin_count,
                static_cast<Monero::PendingTransaction::Priority>(priority), currentSubaddressAccount(), subaddr_indices);
    PendingTransaction * result = new PendingTransaction(ptImpl, this);
    return result;
}

void Wallet::createTransactionAllAsync(const QString &dst_addr, const QString &payment_id,
                               quint32 mixin_count,
                               PendingTransaction::Priority priority)
{
    m_scheduler.run([this, dst_addr, payment_id, mixin_count, priority] {
        PendingTransaction *tx = createTransactionAll(dst_addr, payment_id, mixin_count, priority);
        emit transactionCreated(tx, dst_addr, payment_id, mixin_count);
    });
}

PendingTransaction *Wallet::createTransactionSingle(const QString &key_image, const QString &dst_addr, const size_t outputs,
        PendingTransaction::Priority priority)
{
    Monero::PendingTransaction * ptImpl = m_walletImpl->createTransactionSingle(key_image.toStdString(), dst_addr.toStdString(),
            outputs, static_cast<Monero::PendingTransaction::Priority>(priority));
    PendingTransaction * result = new PendingTransaction(ptImpl, this);
    return result;
}

void Wallet::createTransactionSingleAsync(const QString &key_image, const QString &dst_addr, const size_t outputs,
                                          PendingTransaction::Priority priority)
{
    m_scheduler.run([this, key_image, dst_addr, outputs, priority] {
        PendingTransaction *tx = createTransactionSingle(key_image, dst_addr, outputs, priority);
        emit transactionCreated(tx, dst_addr, "", 0); // todo: return true mixincount
    });
}

PendingTransaction *Wallet::createSweepUnmixableTransaction()
{
    Monero::PendingTransaction * ptImpl = m_walletImpl->createSweepUnmixableTransaction();
    PendingTransaction * result = new PendingTransaction(ptImpl, this);
    return result;
}

void Wallet::createSweepUnmixableTransactionAsync()
{
    m_scheduler.run([this] {
        PendingTransaction *tx = createSweepUnmixableTransaction();
        emit transactionCreated(tx, "", "", 0);
    });
}

UnsignedTransaction * Wallet::loadTxFile(const QString &fileName)
{
    qDebug() << "Trying to sign " << fileName;
    Monero::UnsignedTransaction * ptImpl = m_walletImpl->loadUnsignedTx(fileName.toStdString());
    UnsignedTransaction * result = new UnsignedTransaction(ptImpl, m_walletImpl, this);
    return result;
}

bool Wallet::submitTxFile(const QString &fileName) const
{
    qDebug() << "Trying to submit " << fileName;
    if (!m_walletImpl->submitTransaction(fileName.toStdString()))
        return false;
    // import key images
    return m_walletImpl->importKeyImages(fileName.toStdString() + "_keyImages");
}

void Wallet::commitTransactionAsync(PendingTransaction *t)
{
    m_scheduler.run([this, t] {
        auto txIdList = t->txid();  // retrieve before commit
        emit transactionCommitted(t->commit(), t, txIdList);
    });
}

void Wallet::disposeTransaction(PendingTransaction *t)
{
    m_walletImpl->disposeTransaction(t->m_pimpl);
    delete t;
}

void Wallet::disposeTransaction(UnsignedTransaction *t)
{
    delete t;
}

// @ TODO: QJSValue could probably be QVARIANT
//void Wallet::estimateTransactionFeeAsync(const QString &destination,
//                                         quint64 amount,
//                                         PendingTransaction::Priority priority,
//                                         const QJSValue &callback)
//{
//    m_scheduler.run([this, destination, amount, priority] {
//        const uint64_t fee = m_walletImpl->estimateTransactionFee(
//            {std::make_pair(destination.toStdString(), amount)},
//            static_cast<Monero::PendingTransaction::Priority>(priority));
//        return QJSValueList({QString::fromStdString(Monero::Wallet::displayAmount(fee))});
//    }, callback);
//}

TransactionHistory *Wallet::history() const
{
    return m_history;
}

TransactionHistoryProxyModel *Wallet::historyModel() const
{
    if (!m_historyModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_historyModel = new TransactionHistoryModel(w);
        m_historyModel->setTransactionHistory(this->history());
        m_historySortFilterModel = new TransactionHistoryProxyModel(w);
        m_historySortFilterModel->setSourceModel(m_historyModel);
        m_historySortFilterModel->setSortRole(TransactionHistoryModel::Date);
        m_historySortFilterModel->sort(0, Qt::DescendingOrder);
    }

    return m_historySortFilterModel;
}

TransactionHistoryModel *Wallet::transactionHistoryModel() const
{
    return m_historyModel;
}

AddressBook *Wallet::addressBook() const
{
    return m_addressBook;
}

AddressBookModel *Wallet::addressBookModel() const
{

    if (!m_addressBookModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_addressBookModel = new AddressBookModel(w,m_addressBook);
    }

    return m_addressBookModel;
}

Subaddress *Wallet::subaddress()
{
    return m_subaddress;
}

SubaddressModel *Wallet::subaddressModel()
{
    if (!m_subaddressModel) {
        m_subaddressModel = new SubaddressModel(this, m_subaddress);
    }
    return m_subaddressModel;
}

SubaddressAccount *Wallet::subaddressAccount() const
{
    return m_subaddressAccount;
}

SubaddressAccountModel *Wallet::subaddressAccountModel() const
{
    if (!m_subaddressAccountModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_subaddressAccountModel = new SubaddressAccountModel(w,m_subaddressAccount);
    }
    return m_subaddressAccountModel;
}

Coins *Wallet::coins() const
{
    return m_coins;
}

CoinsModel *Wallet::coinsModel() const
{
    if (!m_coinsModel) {
        Wallet * w = const_cast<Wallet*>(this);
        m_coinsModel = new CoinsModel(w, m_coins);
    }
    return m_coinsModel;
}

QString Wallet::generatePaymentId() const
{
    return QString::fromStdString(Monero::Wallet::genPaymentId());
}

QString Wallet::integratedAddress(const QString &paymentId) const
{
    return QString::fromStdString(m_walletImpl->integratedAddress(paymentId.toStdString()));
}

QString Wallet::paymentId() const
{
    return m_paymentId;
}

void Wallet::setPaymentId(const QString &paymentId)
{
    m_paymentId = paymentId;
}

QString Wallet::getCacheAttribute(const QString &key) const {
    return QString::fromStdString(m_walletImpl->getCacheAttribute(key.toStdString()));
}

bool Wallet::setCacheAttribute(const QString &key, const QString &val)
{
    return m_walletImpl->setCacheAttribute(key.toStdString(), val.toStdString());
}

bool Wallet::setUserNote(const QString &txid, const QString &note)
{
  return m_walletImpl->setUserNote(txid.toStdString(), note.toStdString());
}

QString Wallet::getUserNote(const QString &txid) const
{
  return QString::fromStdString(m_walletImpl->getUserNote(txid.toStdString()));
}

QString Wallet::getTxKey(const QString &txid) const
{
  return QString::fromStdString(m_walletImpl->getTxKey(txid.toStdString()));
}

//void Wallet::getTxKeyAsync(const QString &txid, const QJSValue &callback)
//{
//    m_scheduler.run([this, txid] {
//        return QJSValueList({txid, getTxKey(txid)});
//    }, callback);
//}

QString Wallet::checkTxKey(const QString &txid, const QString &tx_key, const QString &address)
{
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
    bool success = m_walletImpl->checkTxKey(txid.toStdString(), tx_key.toStdString(), address.toStdString(), received, in_pool, confirmations);
    std::string result = std::string(success ? "true" : "false") + "|" + QString::number(received).toStdString() + "|" + std::string(in_pool ? "true" : "false") + "|" + QString::number(confirmations).toStdString();
    return QString::fromStdString(result);
}

QString Wallet::getTxProof(const QString &txid, const QString &address, const QString &message) const
{
    std::string result = m_walletImpl->getTxProof(txid.toStdString(), address.toStdString(), message.toStdString());
    if (result.empty())
        result = "error|" + m_walletImpl->errorString();
    return QString::fromStdString(result);
}

//void Wallet::getTxProofAsync(const QString &txid, const QString &address, const QString &message, const QJSValue &callback)
//{
//    m_scheduler.run([this, txid, address, message] {
//        return QJSValueList({txid, getTxProof(txid, address, message)});
//    }, callback);
//}

TxProofResult Wallet::checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature)
{
    bool good;
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
    bool success = m_walletImpl->checkTxProof(txid.toStdString(), address.toStdString(), message.toStdString(), signature.toStdString(), good, received, in_pool, confirmations);
    return {success, good, received, in_pool, confirmations};
}

Q_INVOKABLE QString Wallet::getSpendProof(const QString &txid, const QString &message) const
{
    std::string result = m_walletImpl->getSpendProof(txid.toStdString(), message.toStdString());
    if (result.empty())
        result = "error|" + m_walletImpl->errorString();
    return QString::fromStdString(result);
}

//void Wallet::getSpendProofAsync(const QString &txid, const QString &message, const QJSValue &callback)
//{
//    m_scheduler.run([this, txid, message] {
//        return QJSValueList({txid, getSpendProof(txid, message)});
//    }, callback);
//}

Q_INVOKABLE QPair<bool, bool> Wallet::checkSpendProof(const QString &txid, const QString &message,
                                                      const QString &signature) const {
    bool good;
    bool success = m_walletImpl->checkSpendProof(txid.toStdString(), message.toStdString(), signature.toStdString(), good);
    return {success, good};
}

QString Wallet::signMessage(const QString &message, bool filename, const QString &address) const
{
  if (filename) {
    QFile file(message);
    uchar *data = NULL;

    try {
      if (!file.open(QIODevice::ReadOnly))
        return "";
      quint64 size = file.size();
      if (size == 0) {
        file.close();
        return QString::fromStdString(m_walletImpl->signMessage(std::string()));
      }
      data = file.map(0, size);
      if (!data) {
        file.close();
        return "";
      }
      std::string signature = m_walletImpl->signMessage(std::string(reinterpret_cast<const char*>(data), size), address.toStdString());
      file.unmap(data);
      file.close();
      return QString::fromStdString(signature);
    }
    catch (const std::exception &e) {
      if (data)
        file.unmap(data);
      file.close();
      return "";
    }
  }
  else {
    return QString::fromStdString(m_walletImpl->signMessage(message.toStdString(), address.toStdString()));
  }
}

bool Wallet::verifySignedMessage(const QString &message, const QString &address, const QString &signature, bool filename) const
{
  if (filename) {
    QFile file(message);
    uchar *data = NULL;

    try {
      if (!file.open(QIODevice::ReadOnly))
        return false;
      quint64 size = file.size();
      if (size == 0) {
        file.close();
        return m_walletImpl->verifySignedMessage(std::string(), address.toStdString(), signature.toStdString());
      }
      data = file.map(0, size);
      if (!data) {
        file.close();
        return false;
      }
      bool ret = m_walletImpl->verifySignedMessage(std::string(reinterpret_cast<const char*>(data), size), address.toStdString(), signature.toStdString());
      file.unmap(data);
      file.close();
      return ret;
    }
    catch (const std::exception &e) {
      if (data)
        file.unmap(data);
      file.close();
      return false;
    }
  }
  else {
    return m_walletImpl->verifySignedMessage(message.toStdString(), address.toStdString(), signature.toStdString());
  }
}

bool Wallet::parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error)
{
   std::string s_address, s_payment_id, s_tx_description, s_recipient_name, s_error;
   std::vector<std::string> s_unknown_parameters;
   bool res= m_walletImpl->parse_uri(uri.toStdString(), s_address, s_payment_id, amount, s_tx_description, s_recipient_name, s_unknown_parameters, s_error);
   if(res)
   {
       address = QString::fromStdString(s_address);
       payment_id = QString::fromStdString(s_payment_id);
       tx_description = QString::fromStdString(s_tx_description);
       recipient_name = QString::fromStdString(s_recipient_name);
       for( const auto &p : s_unknown_parameters )
           unknown_parameters.append(QString::fromStdString(p));
   }
   error = QString::fromStdString(s_error);
   return res;
}

bool Wallet::rescanSpent()
{
    return m_walletImpl->rescanSpent();
}

bool Wallet::useForkRules(quint8 required_version, quint64 earlyBlocks) const
{
    if(m_connectionStatus == Wallet::ConnectionStatus_Disconnected)
        return false;
    try {
        return m_walletImpl->useForkRules(required_version,earlyBlocks);
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return false;
    }
}

void Wallet::setWalletCreationHeight(quint64 height)
{
    m_walletImpl->setRefreshFromBlockHeight(height);
    emit walletCreationHeightChanged();
}

QString Wallet::getDaemonLogPath() const
{
    return QString::fromStdString(m_walletImpl->getDefaultDataDir()) + "/bitmonero.log";
}

bool Wallet::blackballOutput(const QString &amount, const QString &offset)
{
    return m_walletImpl->blackballOutput(amount.toStdString(), offset.toStdString());
}

bool Wallet::blackballOutputs(const QList<QString> &pubkeys, bool add)
{
    std::vector<std::string> std_pubkeys;
    foreach (const QString &pubkey, pubkeys) {
        std_pubkeys.push_back(pubkey.toStdString());
    }
    return m_walletImpl->blackballOutputs(std_pubkeys, add);
}

bool Wallet::blackballOutputs(const QString &filename, bool add)
{
    QFile file(filename);

    try {
        if (!file.open(QIODevice::ReadOnly))
            return false;
        QList<QString> outputs;
        QTextStream in(&file);
        while (!in.atEnd()) {
            outputs.push_back(in.readLine());
        }
        file.close();
        return blackballOutputs(outputs, add);
    }
    catch (const std::exception &e) {
        file.close();
        return false;
    }
}

bool Wallet::unblackballOutput(const QString &amount, const QString &offset)
{
    return m_walletImpl->unblackballOutput(amount.toStdString(), offset.toStdString());
}

QString Wallet::getRing(const QString &key_image)
{
    std::vector<uint64_t> cring;
    if (!m_walletImpl->getRing(key_image.toStdString(), cring))
        return "";
    QString ring = "";
    for (uint64_t out: cring)
    {
        if (!ring.isEmpty())
            ring = ring + " ";
	QString s;
	s.setNum(out);
        ring = ring + s;
    }
    return ring;
}

QString Wallet::getRings(const QString &txid)
{
    std::vector<std::pair<std::string, std::vector<uint64_t>>> crings;
    if (!m_walletImpl->getRings(txid.toStdString(), crings))
        return "";
    QString ring = "";
    for (const auto &cring: crings)
    {
        if (!ring.isEmpty())
            ring = ring + "|";
        ring = ring + QString::fromStdString(cring.first) + " absolute";
        for (uint64_t out: cring.second)
        {
            ring = ring + " ";
	    QString s;
	    s.setNum(out);
            ring = ring + s;
        }
    }
    return ring;
}

bool Wallet::setRing(const QString &key_image, const QString &ring, bool relative)
{
    std::vector<uint64_t> cring;
    QStringList strOuts = ring.split(" ");
    foreach(QString str, strOuts)
    {
        uint64_t out;
	bool ok;
	out = str.toULong(&ok);
	if (ok)
            cring.push_back(out);
    }
    return m_walletImpl->setRing(key_image.toStdString(), cring, relative);
}

void Wallet::segregatePreForkOutputs(bool segregate)
{
    m_walletImpl->segregatePreForkOutputs(segregate);
}

void Wallet::segregationHeight(quint64 height)
{
    m_walletImpl->segregationHeight(height);
}

void Wallet::keyReuseMitigation2(bool mitigation)
{
    m_walletImpl->keyReuseMitigation2(mitigation);
}

void Wallet::onWalletPassphraseNeeded(bool on_device)
{
    emit this->walletPassphraseNeeded(on_device);
}

void Wallet::onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort)
{
    if (m_walletListener != nullptr)
    {
        m_walletListener->onPassphraseEntered(passphrase, enter_on_device, entry_abort);
    }
}

Wallet::Wallet(Monero::Wallet *w, QObject *parent)
    : QObject(parent)
    , m_walletImpl(w)
    , m_history(nullptr)
    , m_historyModel(nullptr)
    , m_addressBook(nullptr)
    , m_addressBookModel(nullptr)
    , m_daemonBlockChainHeight(0)
    , m_daemonBlockChainHeightTtl(DAEMON_BLOCKCHAIN_HEIGHT_CACHE_TTL_SECONDS)
    , m_daemonBlockChainTargetHeight(0)
    , m_daemonBlockChainTargetHeightTtl(DAEMON_BLOCKCHAIN_TARGET_HEIGHT_CACHE_TTL_SECONDS)
    , m_connectionStatus(Wallet::ConnectionStatus_Disconnected)
    , m_connectionStatusTtl(WALLET_CONNECTION_STATUS_CACHE_TTL_SECONDS)
    , m_disconnected(true)
    , m_currentSubaddressAccount(0)
    , m_subaddress(nullptr)
    , m_subaddressModel(nullptr)
    , m_subaddressAccount(nullptr)
    , m_subaddressAccountModel(nullptr)
    , m_coinsModel(nullptr)
    , m_scheduler(this)
{
    m_history = new TransactionHistory(m_walletImpl->history(), this);
    m_addressBook = new AddressBook(m_walletImpl->addressBook(), this);
    m_subaddress = new Subaddress(m_walletImpl->subaddress(), this);
    m_subaddressAccount = new SubaddressAccount(m_walletImpl->subaddressAccount(), this);
    m_coins = new Coins(m_walletImpl->coins(), this);
    m_walletListener = new WalletListenerImpl(this);
    m_walletImpl->setListener(m_walletListener);
    m_currentSubaddressAccount = getCacheAttribute(ATTRIBUTE_SUBADDRESS_ACCOUNT).toUInt();
    // start cache timers
    m_connectionStatusTime.start();
    m_daemonBlockChainHeightTime.start();
    m_daemonBlockChainTargetHeightTime.start();
    m_initialized = false;
    m_connectionStatusRunning = false;
    m_daemonUsername = "";
    m_daemonPassword = "";
}

Wallet::~Wallet()
{
    qDebug() << "~Wallet: Closing wallet";

    m_scheduler.shutdownWaitForFinished();

    delete m_addressBook;
    m_addressBook = NULL;

    delete m_history;
    m_history = NULL;
    delete m_addressBook;
    m_addressBook = NULL;
    delete m_subaddress;
    m_subaddress = NULL;
    delete m_subaddressAccount;
    m_subaddressAccount = NULL;
    delete m_coins;
    m_coins = NULL;
    //Monero::WalletManagerFactory::getWalletManager()->closeWallet(m_walletImpl);
    if(status() == Status_Critical)
        qDebug("Not storing wallet cache");
    else if( m_walletImpl->store(""))
        qDebug("Wallet cache stored successfully");
    else
        qDebug("Error storing wallet cache");
    delete m_walletImpl;
    m_walletImpl = NULL;
    delete m_walletListener;
    m_walletListener = NULL;
    qDebug("m_walletImpl deleted");
}