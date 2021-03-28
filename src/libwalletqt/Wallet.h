// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef WALLET_H
#define WALLET_H

#include <QElapsedTimer>
#include <QObject>
#include <QMutex>
#include <QList>
#include <QtConcurrent/QtConcurrent>

#include "wallet/api/wallet2_api.h" // we need to have an access to the Monero::Wallet::Status enum here;
#include "utils/scheduler.h"
#include "PendingTransaction.h" // we need to have an access to the PendingTransaction::Priority enum here;
#include "UnsignedTransaction.h"
#include "utils/networktype.h"
#include "PassphraseHelper.h"
#include "WalletListenerImpl.h"

namespace Monero {
    struct Wallet; // forward declaration
}

struct TxProof {
    TxProof(QString proof, QString error = "")
        : proof(std::move(proof)), error(std::move(error)){}

    QString proof;
    QString error;
};

struct SubaddressIndex {
    SubaddressIndex(int major, int minor) {
        this->major = major;
        this->minor = minor;
    }

    bool isValid() const {
        return major >= 0 && minor >= 0;
    }

    bool isPrimary() const {
        return major == 0 && minor == 0;
    }

    int major;
    int minor;
};

class TransactionHistory;
class TransactionHistoryModel;
class TransactionHistoryProxyModel;
class AddressBook;
class AddressBookModel;
class Subaddress;
class SubaddressModel;
class SubaddressAccount;
class SubaddressAccountModel;
class Coins;
class CoinsModel;

struct TxProofResult {
    TxProofResult(bool success, bool good, uint64_t received, bool in_pool, uint64_t confirmations)
      : success(success), good(good), received(received), in_pool(in_pool), confirmations(confirmations){}

    bool success;
    bool good;
    uint64_t received;
    bool in_pool;
    uint64_t confirmations;
};

class Wallet : public QObject, public PassprasePrompter
{
Q_OBJECT
    Q_PROPERTY(bool disconnected READ disconnected NOTIFY disconnectedChanged)
    Q_PROPERTY(bool refreshing READ refreshing NOTIFY refreshingChanged)
    Q_PROPERTY(QString seed READ getSeed)
    Q_PROPERTY(QString seedLanguage READ getSeedLanguage)
    Q_PROPERTY(Status status READ status)
    Q_PROPERTY(NetworkType::Type nettype READ nettype)
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus)
    Q_PROPERTY(quint32 currentSubaddressAccount READ currentSubaddressAccount NOTIFY currentSubaddressAccountChanged)
    Q_PROPERTY(bool synchronized READ synchronized)
    Q_PROPERTY(QString errorString READ errorString)
    Q_PROPERTY(TransactionHistory * history READ history)
    Q_PROPERTY(QString paymentId READ paymentId WRITE setPaymentId)
    Q_PROPERTY(TransactionHistoryProxyModel * historyModel READ historyModel NOTIFY historyModelChanged)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(AddressBookModel * addressBookModel READ addressBookModel)
    Q_PROPERTY(AddressBook * addressBook READ addressBook NOTIFY addressBookChanged)
    Q_PROPERTY(SubaddressModel * subaddressModel READ subaddressModel)
    Q_PROPERTY(Subaddress * subaddress READ subaddress)
    Q_PROPERTY(SubaddressAccountModel * subaddressAccountModel READ subaddressAccountModel)
    Q_PROPERTY(SubaddressAccount * subaddressAccount READ subaddressAccount)
    Q_PROPERTY(bool viewOnly READ viewOnly)
    Q_PROPERTY(QString secretViewKey READ getSecretViewKey)
    Q_PROPERTY(QString publicViewKey READ getPublicViewKey)
    Q_PROPERTY(QString secretSpendKey READ getSecretSpendKey)
    Q_PROPERTY(QString publicSpendKey READ getPublicSpendKey)
    Q_PROPERTY(QString daemonLogPath READ getDaemonLogPath CONSTANT)
    Q_PROPERTY(QString proxyAddress READ getProxyAddress WRITE setProxyAddress NOTIFY proxyAddressChanged)
    Q_PROPERTY(quint64 walletCreationHeight READ getWalletCreationHeight WRITE setWalletCreationHeight NOTIFY walletCreationHeightChanged)

public:


    enum Status {
        Status_Ok       = Monero::Wallet::Status_Ok,
        Status_Error    = Monero::Wallet::Status_Error,
        Status_Critical = Monero::Wallet::Status_Critical
    };

    Q_ENUM(Status)

    enum ConnectionStatus {
        ConnectionStatus_Disconnected    = Monero::Wallet::ConnectionStatus_Disconnected,
        ConnectionStatus_Connected       = Monero::Wallet::ConnectionStatus_Connected,
        ConnectionStatus_WrongVersion    = Monero::Wallet::ConnectionStatus_WrongVersion,
        ConnectionStatus_Connecting
    };

    Q_ENUM(ConnectionStatus)

    //! return connection status
    ConnectionStatus connectionStatus() const;

    //! returns mnemonic seed
    QString getSeed() const;

    //! returns seed language
    QString getSeedLanguage() const;

    //! set seed language
    Q_INVOKABLE void setSeedLanguage(const QString &lang);

    //! returns last operation's status
    Status status() const;

    //! returns network type of the wallet.
    NetworkType::Type nettype() const;

    //! returns true if wallet was ever synchronized
    bool synchronized() const;


    //! returns last operation's error message
    QString errorString() const;

    //! changes the password using existing parameters (path, seed, seed lang)
    Q_INVOKABLE bool setPassword(const QString &password);

    //! get current wallet password
    Q_INVOKABLE QString getPassword();

    //! returns wallet's public address
    Q_INVOKABLE QString address(quint32 accountIndex, quint32 addressIndex) const;

    //! returns the subaddress index of the address
    Q_INVOKABLE SubaddressIndex subaddressIndex(const QString &address) const;

    //! returns wallet file's path
    QString path() const;

    //! saves wallet to the file by given path
    //! empty path stores in current location
    Q_INVOKABLE void store(const QString &path = "");
   // Q_INVOKABLE void storeAsync(const QJSValue &callback, const QString &path = "");

    //! initializes wallet asynchronously
    Q_INVOKABLE void initAsync(
            const QString &daemonAddress,
            bool trustedDaemon = false,
            quint64 upperTransactionLimit = 0,
            bool isRecovering = false,
            bool isRecoveringFromDevice = false,
            quint64 restoreHeight = 0,
            const QString &proxyAddress = "");

    // Set daemon rpc user/pass
    Q_INVOKABLE void setDaemonLogin(const QString &daemonUsername = "", const QString &daemonPassword = "");

    //! create a view only wallet
    Q_INVOKABLE bool createViewOnly(const QString &path, const QString &password) const;

    //! connects to daemon
    Q_INVOKABLE bool connectToDaemon();

    //! set connect to daemon timeout
    Q_INVOKABLE void setConnectionTimeout(int timeout);

    //! indicates if daemon is trusted
    Q_INVOKABLE void setTrustedDaemon(bool arg);

    //! indicates if ssl should be used to connect to daemon
    Q_INVOKABLE void setUseSSL(bool ssl);

    //! returns balance
    Q_INVOKABLE quint64 balance() const;
    Q_INVOKABLE quint64 balance(quint32 accountIndex) const;
    Q_INVOKABLE quint64 balanceAll() const;

    //! returns unlocked balance
    Q_INVOKABLE quint64 unlockedBalance() const;
    Q_INVOKABLE quint64 unlockedBalance(quint32 accountIndex) const;
    Q_INVOKABLE quint64 unlockedBalanceAll() const;

    //! account/address management
    quint32 currentSubaddressAccount() const;
    Q_INVOKABLE void switchSubaddressAccount(quint32 accountIndex);
    Q_INVOKABLE void addSubaddressAccount(const QString& label);
    Q_INVOKABLE quint32 numSubaddressAccounts() const;
    Q_INVOKABLE quint32 numSubaddresses(quint32 accountIndex) const;
    Q_INVOKABLE void addSubaddress(const QString& label);
    Q_INVOKABLE QString getSubaddressLabel(quint32 accountIndex, quint32 addressIndex) const;
    Q_INVOKABLE void setSubaddressLabel(quint32 accountIndex, quint32 addressIndex, const QString &label);
    Q_INVOKABLE void deviceShowAddressAsync(quint32 accountIndex, quint32 addressIndex, const QString &paymentId);

    //! hw-device backed wallets
    Q_INVOKABLE bool isHwBacked() const;
    Q_INVOKABLE bool isLedger() const;
    Q_INVOKABLE bool isTrezor() const;

    //! returns if view only wallet
    Q_INVOKABLE bool viewOnly() const;

    Q_INVOKABLE void refreshHeightAsync();

    //! export/import key images
    Q_INVOKABLE bool exportKeyImages(const QString& path, bool all = false);
    Q_INVOKABLE bool importKeyImages(const QString& path);

    //! export/import outputs
    Q_INVOKABLE bool exportOutputs(const QString& path, bool all = false);
    Q_INVOKABLE bool importOutputs(const QString& path);

    //! import a transaction
    Q_INVOKABLE bool importTransaction(const QString& txid, const QVector<quint64>& output_indeces, quint64 height, quint64 timestamp, bool miner_tx, bool pool, bool double_spend_seen);

    Q_INVOKABLE QString printBlockchain();
    Q_INVOKABLE QString printTransfers();
    Q_INVOKABLE QString printPayments();
    Q_INVOKABLE QString printUnconfirmedPayments();
    Q_INVOKABLE QString printConfirmedTransferDetails();
    Q_INVOKABLE QString printUnconfirmedTransferDetails();
    Q_INVOKABLE QString printPubKeys();
    Q_INVOKABLE QString printTxNotes();
    Q_INVOKABLE QString printSubaddresses();
    Q_INVOKABLE QString printSubaddressLabels();
    Q_INVOKABLE QString printAdditionalTxKeys();
    Q_INVOKABLE QString printAttributes();
    Q_INVOKABLE QString printKeyImages();
    Q_INVOKABLE QString printAccountTags();
    Q_INVOKABLE QString printTxKeys();
    Q_INVOKABLE QString printAddressBook();
    Q_INVOKABLE QString printScannedPoolTxs();

    //! refreshes the wallet
    Q_INVOKABLE bool refresh(bool historyAndSubaddresses = false);

    // pause/resume refresh
    Q_INVOKABLE void startRefresh();
    Q_INVOKABLE void pauseRefresh();

    //! returns current wallet's block height
    //! (can be less than daemon's blockchain height when wallet sync in progress)
    Q_INVOKABLE quint64 blockChainHeight() const;

    //! returns daemon's blockchain height
    Q_INVOKABLE quint64 daemonBlockChainHeight() const;

    //! returns daemon's blockchain target height
    Q_INVOKABLE quint64 daemonBlockChainTargetHeight() const;

    //! creates transaction
    Q_INVOKABLE PendingTransaction * createTransaction(const QString &dst_addr, const QString &payment_id,
                                                       quint64 amount, quint32 mixin_count,
                                                       PendingTransaction::Priority priority);

    //! creates async transaction
    Q_INVOKABLE void createTransactionAsync(const QString &dst_addr, const QString &payment_id,
                                            quint64 amount, quint32 mixin_count,
                                            PendingTransaction::Priority priority);

    //! creates multi-destination transaction
    Q_INVOKABLE PendingTransaction * createTransactionMultiDest(const QVector<QString> &dst_addr, const QVector<quint64> &amount,
                                                                PendingTransaction::Priority priority);

    //! creates async multi-destination transaction
    Q_INVOKABLE void createTransactionMultiDestAsync(const QVector<QString> &dst_addr, const QVector<quint64> &amount,
                                                     PendingTransaction::Priority priority);


    //! creates transaction with all outputs
    Q_INVOKABLE PendingTransaction * createTransactionAll(const QString &dst_addr, const QString &payment_id,
                                                          quint32 mixin_count, PendingTransaction::Priority priority);

    //! creates async transaction with all outputs
    Q_INVOKABLE void createTransactionAllAsync(const QString &dst_addr, const QString &payment_id,
                                               quint32 mixin_count, PendingTransaction::Priority priority);

    //! creates transaction with single input
    Q_INVOKABLE PendingTransaction * createTransactionSingle(const QString &key_image, const QString &dst_addr,
            size_t outputs, PendingTransaction::Priority priority);

    //! creates async transaction with single input
    Q_INVOKABLE void createTransactionSingleAsync(const QString &key_image, const QString &dst_addr,
            size_t outputs, PendingTransaction::Priority priority);

    //! creates sweep unmixable transaction
    Q_INVOKABLE PendingTransaction * createSweepUnmixableTransaction();

    //! creates async sweep unmixable transaction
    Q_INVOKABLE void createSweepUnmixableTransactionAsync();

    //! Sign a transfer from file
    Q_INVOKABLE UnsignedTransaction * loadTxFile(const QString &fileName);

    //! Load an unsigned transaction from a base64 encoded string
    Q_INVOKABLE UnsignedTransaction * loadTxFromBase64Str(const QString &unsigned_tx);

    //! Load a signed transaction from file
    Q_INVOKABLE PendingTransaction * loadSignedTxFile(const QString &fileName);

    //! Submit a transfer from file
    Q_INVOKABLE bool submitTxFile(const QString &fileName) const;

    //! asynchronous transaction commit
    Q_INVOKABLE void commitTransactionAsync(PendingTransaction * t);

    //! deletes transaction and frees memory
    Q_INVOKABLE void disposeTransaction(PendingTransaction * t);

    //! deletes unsigned transaction and frees memory
    Q_INVOKABLE void disposeTransaction(UnsignedTransaction * t);

//    Q_INVOKABLE void estimateTransactionFeeAsync(const QString &destination,
//                                                 quint64 amount,
//                                                 PendingTransaction::Priority priority,
//                                                 const QJSValue &callback);

    //! returns transaction history
    TransactionHistory * history() const;

    //! returns transaction history model
    TransactionHistoryProxyModel *historyModel() const;

    //! returns transaction history model (the real one)
    TransactionHistoryModel *transactionHistoryModel() const;

    //! returns Address book
    AddressBook *addressBook() const;

    //! returns adress book model
    AddressBookModel *addressBookModel() const;

    //! returns subaddress
    Subaddress *subaddress();

    //! returns subadress model
    SubaddressModel *subaddressModel();

    //! returns subaddress account
    SubaddressAccount *subaddressAccount() const;

    //! returns subadress account model
    SubaddressAccountModel *subaddressAccountModel() const;

    //! returns coins
    Coins *coins() const;

    //! return coins model
    CoinsModel *coinsModel() const;

    //! generate payment id
    Q_INVOKABLE QString generatePaymentId() const;

    //! integrated address
    Q_INVOKABLE QString integratedAddress(const QString &paymentId) const;

    //! signing a message
    Q_INVOKABLE QString signMessage(const QString &message, bool filename = false, const QString &address = "") const;

    //! verify a signed message
    Q_INVOKABLE bool verifySignedMessage(const QString &message, const QString &address, const QString &signature, bool filename = false) const;

    //! Parse URI
    Q_INVOKABLE bool parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error);

    //! saved payment id
    QString paymentId() const;

    void setPaymentId(const QString &paymentId);

    //! Namespace your cacheAttribute keys to avoid collisions
    Q_INVOKABLE bool setCacheAttribute(const QString &key, const QString &val);
    Q_INVOKABLE QString getCacheAttribute(const QString &key) const;

    Q_INVOKABLE bool setUserNote(const QString &txid, const QString &note);
    Q_INVOKABLE QString getUserNote(const QString &txid) const;
    Q_INVOKABLE QString getTxKey(const QString &txid) const;
    //Q_INVOKABLE void getTxKeyAsync(const QString &txid, const QJSValue &callback);
    Q_INVOKABLE QString checkTxKey(const QString &txid, const QString &tx_key, const QString &address);
    Q_INVOKABLE TxProof getTxProof(const QString &txid, const QString &address, const QString &message) const;
   // Q_INVOKABLE void getTxProofAsync(const QString &txid, const QString &address, const QString &message, const QJSValue &callback);
    //Q_INVOKABLE QString checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature);
    Q_INVOKABLE TxProofResult checkTxProof(const QString &txid, const QString &address, const QString &message, const QString &signature);
    Q_INVOKABLE TxProof getSpendProof(const QString &txid, const QString &message) const;
   // Q_INVOKABLE void getSpendProofAsync(const QString &txid, const QString &message, const QJSValue &callback);
    Q_INVOKABLE QPair<bool, bool> checkSpendProof(const QString &txid, const QString &message, const QString &signature) const;
    // Rescan spent outputs
    Q_INVOKABLE bool rescanSpent();

    // check if fork rules should be used
    Q_INVOKABLE bool useForkRules(quint8 version, quint64 earlyBlocks = 0) const;

    //! Get wallet keys
    QString getSecretViewKey() const {return QString::fromStdString(m_walletImpl->secretViewKey());}
    QString getPublicViewKey() const {return QString::fromStdString(m_walletImpl->publicViewKey());}
    QString getSecretSpendKey() const {return QString::fromStdString(m_walletImpl->secretSpendKey());}
    QString getPublicSpendKey() const {return QString::fromStdString(m_walletImpl->publicSpendKey());}

    quint64 getWalletCreationHeight() const {return m_walletImpl->getRefreshFromBlockHeight();}
    void setWalletCreationHeight(quint64 height);

    QString getDaemonLogPath() const;
    QString getWalletLogPath() const;

    // Blackalled outputs
    Q_INVOKABLE bool blackballOutput(const QString &amount, const QString &offset);
    Q_INVOKABLE bool blackballOutputs(const QList<QString> &outputs, bool add);
    Q_INVOKABLE bool blackballOutputs(const QString &filename, bool add);
    Q_INVOKABLE bool unblackballOutput(const QString &amount, const QString &offset);

    // Rings
    Q_INVOKABLE QString getRing(const QString &key_image);
    Q_INVOKABLE QString getRings(const QString &txid);
    Q_INVOKABLE bool setRing(const QString &key_image, const QString &ring, bool relative);

    // key reuse mitigation options
    Q_INVOKABLE void segregatePreForkOutputs(bool segregate);
    Q_INVOKABLE void segregationHeight(quint64 height);
    Q_INVOKABLE void keyReuseMitigation2(bool mitigation);

    // Passphrase entry for hardware wallets
    Q_INVOKABLE void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort=false);
    virtual void onWalletPassphraseNeeded(bool on_device) override;

    Q_INVOKABLE quint64 getBytesReceived() const;
    Q_INVOKABLE quint64 getBytesSent() const;

    // return as json object
    QJsonObject toJsonObject();

    // TODO: setListenter() when it implemented in API
signals:
    // emitted on every event happened with wallet
    // (money sent/received, new block)
    void updated();

    // emitted when refresh process finished (could take a long time)
    // signalling only after we
    void refreshed(bool success);

    void moneySpent(const QString &txId, quint64 amount);
    void moneyReceived(const QString &txId, quint64 amount);
    void unconfirmedMoneyReceived(const QString &txId, quint64 amount);
    void newBlock(quint64 height, quint64 targetHeight);
    void addressBookChanged() const;
    void historyModelChanged() const;
    void walletCreationHeightChanged();
    void deviceButtonRequest(quint64 buttonCode);
    void deviceButtonPressed();
    void walletPassphraseNeeded(bool onDevice);
    void transactionCommitted(bool status, PendingTransaction *t, const QStringList& txid);
    void heightRefreshed(quint64 walletHeight, quint64 daemonHeight, quint64 targetHeight) const;
    void deviceShowAddressShowed();

    // emitted when transaction is created async
    void transactionCreated(PendingTransaction * transaction, QVector<QString> address);

    void connectionStatusChanged(int status) const;
    void currentSubaddressAccountChanged() const;
    void disconnectedChanged() const;
    void proxyAddressChanged() const;
    void refreshingChanged() const;

private:
    Wallet(QObject * parent = nullptr);
    Wallet(Monero::Wallet *w, QObject * parent = 0);
    ~Wallet();

    //! initializes wallet
    bool init(
            const QString &daemonAddress,
            bool trustedDaemon,
            quint64 upperTransactionLimit,
            bool isRecovering,
            bool isRecoveringFromDevice,
            quint64 restoreHeight,
            const QString& proxyAddress);

    bool disconnected() const;
    bool refreshing() const;
    void refreshingSet(bool value);
    void onRefreshed(bool success);

    void setConnectionStatus(ConnectionStatus value);
    QString getProxyAddress() const;
    void setProxyAddress(QString address);
    void startRefreshThread();

private:
    friend class WalletManager;
    friend class WalletListenerImpl;
    //! libwallet's
    Monero::Wallet * m_walletImpl;
    // history lifetime managed by wallet;
    TransactionHistory * m_history;
    // Used for UI history view
    mutable TransactionHistoryModel * m_historyModel;
    mutable TransactionHistoryProxyModel * m_historySortFilterModel;
    QString m_paymentId;
    AddressBook * m_addressBook;
    mutable AddressBookModel * m_addressBookModel;
    mutable QElapsedTimer m_daemonBlockChainHeightTime;
    mutable quint64 m_daemonBlockChainHeight;
    int     m_daemonBlockChainHeightTtl;
    mutable QElapsedTimer m_daemonBlockChainTargetHeightTime;
    mutable quint64 m_daemonBlockChainTargetHeight;
    int     m_daemonBlockChainTargetHeightTtl;
    mutable ConnectionStatus m_connectionStatus;
    int     m_connectionStatusTtl;
    mutable QElapsedTimer m_connectionStatusTime;
    bool m_disconnected;
    mutable bool    m_initialized;
    uint32_t m_currentSubaddressAccount;
    Subaddress * m_subaddress;
    mutable SubaddressModel * m_subaddressModel;
    SubaddressAccount * m_subaddressAccount;
    mutable SubaddressAccountModel * m_subaddressAccountModel;
    Coins * m_coins;
    mutable CoinsModel * m_coinsModel;
    QMutex m_asyncMutex;
    QMutex m_connectionStatusMutex;
    bool m_connectionStatusRunning;
    QString m_daemonUsername;
    QString m_daemonPassword;
    QString m_proxyAddress;
    mutable QMutex m_proxyMutex;
    std::atomic<bool> m_refreshNow;
    std::atomic<bool> m_refreshEnabled;
    std::atomic<bool> m_refreshing;
    WalletListenerImpl *m_walletListener;
    FutureScheduler m_scheduler;
    int m_connectionTimeout = 30;
    bool m_useSSL;
};



#endif // WALLET_H
