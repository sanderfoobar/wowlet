// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QPixmap>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QGroupBox>
#include <QFileDialog>

#include "mainwindow.h"
#include "dialog/txconfdialog.h"
#include "dialog/txconfadvdialog.h"
#include "dialog/debuginfodialog.h"
#include "dialog/walletinfodialog.h"
#include "dialog/torinfodialog.h"
#include "dialog/viewonlydialog.h"
#include "dialog/broadcasttxdialog.h"
#include "dialog/tximportdialog.h"
#include "dialog/passworddialog.h"
#include "dialog/balancedialog.h"
#include "dialog/WalletCacheDebugDialog.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "utils/ColorScheme.h"

// libwalletqt
#include "libwalletqt/AddressBook.h"

MainWindow * MainWindow::pMainWindow = nullptr;

MainWindow::MainWindow(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_ctx(ctx)
{
    pMainWindow = this;
    ui->setupUi(this);

    m_windowSettings = new Settings(this);
    m_aboutDialog = new AboutDialog(this);
    m_windowCalc = new CalcWindow(this);

    // light/dark theme
    m_skins.insert("Native", "");
    this->loadSkins();
    QString skin = config()->get(Config::skin).toString();
    qApp->setStyleSheet(m_skins[skin]);

    this->screenDpiRef = 128;
    this->screenGeo = QApplication::primaryScreen()->availableGeometry();
    this->screenRect = QGuiApplication::primaryScreen()->geometry();
    this->screenDpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    this->screenDpiPhysical = QGuiApplication::primaryScreen()->physicalDotsPerInch();
    this->screenRatio = this->screenDpiPhysical / this->screenDpiRef;
    qInfo() << QString("%1x%2 (%3 DPI)").arg(this->screenRect.width()).arg(this->screenRect.height()).arg(this->screenDpi);

    this->restoreGeo();

    this->create_status_bar();

    // Bootstrap Tor/websockets
    m_ctx->initTor();
    m_ctx->initWS();

    // update statusbar
    connect(m_ctx->tor, &Tor::connectionStateChanged, [this](bool connected){
        connected ? m_statusBtnTor->setIcon(QIcon(":/assets/images/tor_logo.png"))
                  : m_statusBtnTor->setIcon(QIcon(":/assets/images/tor_logo_disabled.png"));});
    connect(m_ctx->nodes, &Nodes::updateStatus, [=](const QString &msg){
        this->setStatusText(msg);
    });

    // menu connects
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::menuQuitClicked);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::menuSettingsClicked);
    connect(ui->actionCalculator, &QAction::triggered, this, &MainWindow::showCalcWindow);
    connect(ui->actionPay_to_many, &QAction::triggered, this, &MainWindow::payToMany);
    connect(ui->actionWallet_cache_debug, &QAction::triggered, this, &MainWindow::showWalletCacheDebugDialog);

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    ui->actionCreateDesktopEntry->setDisabled(true);
#endif
    connect(ui->actionCreateDesktopEntry, &QAction::triggered, [=]{
        auto msg = Utils::xdgDesktopEntryRegister() ? "Desktop entry created" : "Desktop entry not created due to an error.";
        QMessageBox::information(this, "Desktop entry", msg);
    });
    connect(ui->actionReport_bug, &QAction::triggered, [this](){
        QMessageBox::information(this, "Reporting Bugs",
                                 "<body>Please report any bugs as issues on our git repo:<br>\n"
                                 "<a href=\"https://git.wownero.com/wowlet/wowlet/issues\" style=\"color: #33A4DF\">https://git.wownero.com/wowlet/wowlet/issues</a><br/><br/>"
                                 "\n"
                                 "Before reporting a bug, upgrade to the most recent version of WOWlet "
                                 "(latest release or git HEAD), and include the version number in your report. "
                                 "Try to explain not only what the bug is, but how it occurs.</body>");
    });
    connect(ui->actionShow_debug_info, &QAction::triggered, this, &MainWindow::showDebugInfo);
    connect(ui->actionOfficialWebsite, &QAction::triggered, [=] { Utils::externalLinkWarning(this, "https://wownero.org"); });

#if defined(Q_OS_LINUX)
    // system tray
    m_trayIcon = new QSystemTrayIcon(QIcon(":/assets/images/appicons/64x64.png"));
    m_trayIcon->show();

    m_trayActionCalc = new QAction("Calc", this);
    m_trayActionCalc->setStatusTip("Calculator");

    m_trayActionSend = new QAction("Send", this);
    m_trayActionSend->setStatusTip("Send WOW payment");

    m_trayActionHistory = new QAction("History", this);
    m_trayActionHistory->setStatusTip("View incoming transfers");

    m_trayActionExit = new QAction("Quit", this);
    m_trayActionExit->setStatusTip("Exit application");

    m_trayMenu.addAction(m_trayActionSend);
    m_trayMenu.addAction(m_trayActionHistory);
    m_trayMenu.addAction(m_trayActionCalc);
    m_trayMenu.addAction(m_trayActionExit);
    m_trayIcon->setContextMenu(&m_trayMenu);

    // @TODO: only init tray *after* boot
    connect(m_trayActionCalc, &QAction::triggered, this, &MainWindow::showCalcWindow);
    connect(m_trayActionSend, &QAction::triggered, this, &MainWindow::showSendTab);
    connect(m_trayActionHistory, &QAction::triggered, this, &MainWindow::showHistoryTab);
    connect(m_trayActionExit, &QAction::triggered, this, &QMainWindow::close);
#endif

    // ticker widgets
    m_tickerWOW = new TickerWidget(this, "WOW");
    ui->tickerLayout->addWidget(m_tickerWOW);

    m_tickerSAT = new TickerWidget(this, "Satoshi");
    m_tickerSAT->hidePct(true);
    ui->tickerLayout->addWidget(m_tickerSAT);

    m_tickerBTC = new TickerWidget(this, "BTC");
    ui->tickerLayout->addWidget(m_tickerBTC);

    m_tickerXMR = new TickerWidget(this, "XMR");
    ui->tickerLayout->addWidget(m_tickerXMR);

    m_balanceWidget = new TickerWidget(this, "Balance");
    m_balanceWidget->hidePct(true);
    ui->fiatTickerLayout->addWidget(m_balanceWidget);

    connect(m_tickerWOW, &TickerWidget::reload, this, &MainWindow::onUpdateWowWidget);
    connect(m_tickerBTC, &TickerWidget::reload, this, &MainWindow::onUpdateBTCWidget);
    connect(m_tickerSAT, &TickerWidget::reload, this, &MainWindow::onUpdateSATWidget);
    connect(m_tickerSAT, &TickerWidget::reload, this, &MainWindow::onUpdateXMRWidget);
    connect(m_balanceWidget, &TickerWidget::reload, this, &MainWindow::onUpdateFiatBalanceWidget);

    // Send widget
    connect(ui->sendWidget, &SendWidget::createTransaction, m_ctx, QOverload<const QString, quint64, const QString, bool>::of(&AppContext::onCreateTransaction));
    connect(ui->sendWidget, &SendWidget::createTransactionMultiDest, m_ctx, &AppContext::onCreateTransactionMultiDest);

    // Nodes
    connect(m_ctx->nodes, &Nodes::nodeExhausted, this, &MainWindow::showNodeExhaustedMessage);
    connect(m_ctx->nodes, &Nodes::WSNodeExhausted, this, &MainWindow::showWSNodeExhaustedMessage);

    // XMRig
#ifdef HAS_XMRIG
    m_xmrig = new XMRigWidget(m_ctx, this);
    ui->xmrRigLayout->addWidget(m_xmrig);
    connect(m_ctx->XMRig, &XmRig::output, m_xmrig, &XMRigWidget::onProcessOutput);
    connect(m_ctx->XMRig, &XmRig::error, m_xmrig, &XMRigWidget::onProcessError);
    connect(m_ctx->XMRig, &XmRig::hashrate, m_xmrig, &XMRigWidget::onHashrate);

    connect(m_ctx, &AppContext::walletClosed, m_xmrig, &XMRigWidget::onWalletClosed);
    connect(m_ctx, &AppContext::walletOpened, m_xmrig, &XMRigWidget::onWalletOpened);
    connect(m_ctx, &AppContext::XMRigDownloads, m_xmrig, &XMRigWidget::onDownloads);

    connect(m_xmrig, &XMRigWidget::miningStarted, [=]{ m_ctx->setWindowTitle(true); });
    connect(m_xmrig, &XMRigWidget::miningEnded, [=]{ m_ctx->setWindowTitle(false); });
#else
    ui->tabWidget->setTabVisible(Tabs::XMRIG, false);
#endif

    connect(ui->ccsWidget, &CCSWidget::selected, this, &MainWindow::showSendScreen);
    connect(m_ctx, &AppContext::ccsUpdated, ui->ccsWidget->model(), &CCSModel::updateEntries);
    connect(m_ctx, &AppContext::redditUpdated, ui->redditWidget->model(), &RedditModel::updatePosts);
    connect(m_ctx, &AppContext::suchWowUpdated, ui->suchWowWidget, &SuchWowWidget::onWS);
    connect(ui->suchWowWidget, &SuchWowWidget::donate, this, &MainWindow::suchDonate);

    connect(ui->redditWidget, &RedditWidget::setStatusText, this, &MainWindow::setStatusText);

    connect(ui->tabHomeWidget, &QTabWidget::currentChanged, [](int index){
        config()->set(Config::homeWidget, TabsHome(index));
    });

    connect(m_ctx, &AppContext::donationNag, [=]{
        auto msg = "WOWlet is a 100% community-sponsored endeavor. Please consider supporting "
                   "the project financially. Get rid of this message by donating any amount.";
        int ret = QMessageBox::information(this, "Donate to WOWlet", msg, QMessageBox::Yes, QMessageBox::No);
        switch (ret) {
            case QMessageBox::Yes:
                this->donateButtonClicked();
            case QMessageBox::No:
                break;
            default:
                return;
        }
    });

    // libwalletqt
    connect(m_ctx, &AppContext::walletClosed, [this]{
        this->onWalletClosed();
    });
    connect(m_ctx, &AppContext::walletClosed, ui->sendWidget, &SendWidget::onWalletClosed);
    connect(m_ctx, &AppContext::balanceUpdated, this, &MainWindow::onBalanceUpdated);
    connect(m_ctx, &AppContext::walletOpened, this, &MainWindow::onWalletOpened);
    connect(m_ctx, &AppContext::walletOpenedError, this, &MainWindow::onWalletOpenedError);
    connect(m_ctx, &AppContext::walletCreatedError, this, &MainWindow::onWalletCreatedError);
    connect(m_ctx, &AppContext::walletCreated, this, &MainWindow::onWalletCreated);
    connect(m_ctx, &AppContext::synchronized, this, &MainWindow::onSynchronized);
    connect(m_ctx, &AppContext::blockchainSync, this, &MainWindow::onBlockchainSync);
    connect(m_ctx, &AppContext::refreshSync, this, &MainWindow::onRefreshSync);
    connect(m_ctx, &AppContext::createTransactionError, this, &MainWindow::onCreateTransactionError);
    connect(m_ctx, &AppContext::createTransactionSuccess, this, &MainWindow::onCreateTransactionSuccess);
    connect(m_ctx, &AppContext::transactionCommitted, this, &MainWindow::onTransactionCommitted);
    connect(m_ctx, &AppContext::walletOpenPasswordNeeded, this, &MainWindow::onWalletOpenPasswordRequired);

    // Send
    connect(m_ctx, &AppContext::initiateTransaction, ui->sendWidget, &SendWidget::onInitiateTransaction);
    connect(m_ctx, &AppContext::endTransaction, ui->sendWidget, &SendWidget::onEndTransaction);

    connect(m_ctx, &AppContext::initiateTransaction, [this]{
        m_statusDots = 0;
        m_constructingTransaction = true;
        m_txTimer.start(1000);
    });
    connect(m_ctx, &AppContext::endTransaction, [this]{
        // Todo: endTransaction can fail to fire when the node is switched during tx creation
        m_constructingTransaction = false;
        m_txTimer.stop();
        this->setStatusText(m_statusText);
    });
    connect(&m_txTimer, &QTimer::timeout, [this]{
        m_statusLabelStatus->setText("Constructing transaction" + this->statusDots());
    });


    // testnet/stagenet warning
    auto worthlessWarning = QString("WOWlet is currently running in %1 mode. This is meant "
                                    "for developers only. Your coins are WORTHLESS.");
    if(m_ctx->networkType == NetworkType::STAGENET) {
        if (config()->get(Config::warnOnStagenet).toBool()) {
            QMessageBox::warning(this, "Warning", worthlessWarning.arg("stagenet"));
            config()->set(Config::warnOnStagenet, false);
        }
    }
    else if(m_ctx->networkType == NetworkType::TESTNET){
        if (config()->get(Config::warnOnTestnet).toBool()) {
            QMessageBox::warning(this, "Warning", worthlessWarning.arg("testnet"));
            config()->set(Config::warnOnTestnet, false);
        }
    }

    if(config()->get(Config::warnOnAlpha).toBool()) {
        QString warning = "WOWlet is currently in beta.\n\nPlease report any bugs "
                          "you encounter on our Git repository, IRC freenode #wownero or on /r/Wownero.";
        QMessageBox::warning(this, "Beta Warning", warning);
        config()->set(Config::warnOnAlpha, false);
    }

    // settings connects
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, this, &MainWindow::onUpdateFiatBalanceWidget);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, m_ctx, &AppContext::onPreferredFiatCurrencyChanged);
    connect(m_windowSettings, &Settings::preferredFiatCurrencyChanged, ui->sendWidget, QOverload<>::of(&SendWidget::onPreferredFiatCurrencyChanged));

    // Skin
    connect(m_windowSettings, &Settings::skinChanged, this, &MainWindow::skinChanged);

    // Wizard
    connect(this, &MainWindow::closed, [=]{
        if(m_wizard != nullptr)
            m_wizard->close();
    });

    // Receive
    connect(ui->receiveWidget, &ReceiveWidget::generateSubaddress, [=]() {
        m_ctx->currentWallet->subaddress()->addRow( m_ctx->currentWallet->currentSubaddressAccount(), "");
    });
    connect(ui->receiveWidget, &ReceiveWidget::showTransactions, [this](const QString &text) {
        ui->historyWidget->setSearchText(text);
        ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
    });

    // History
    connect(ui->historyWidget, &HistoryWidget::viewOnBlockExplorer, this, &MainWindow::onViewOnBlockExplorer);
    connect(ui->historyWidget, &HistoryWidget::resendTransaction, this, &MainWindow::onResendTransaction);
    connect(m_ctx, &AppContext::walletRefreshed, ui->historyWidget, &HistoryWidget::onWalletRefreshed);
    connect(m_ctx, &AppContext::walletOpened, ui->historyWidget, &HistoryWidget::onWalletOpened);

    // Contacts
    connect(ui->contactWidget, &ContactsWidget::fillAddress, ui->sendWidget, &SendWidget::fillAddress);

    // Open alias
    connect(ui->sendWidget, &SendWidget::resolveOpenAlias, m_ctx, &AppContext::onOpenAliasResolve);
    connect(m_ctx, &AppContext::openAliasResolveError, ui->sendWidget, &SendWidget::onOpenAliasResolveError);
    connect(m_ctx, &AppContext::openAliasResolved, ui->sendWidget, &SendWidget::onOpenAliasResolved);

    // Coins
    connect(ui->coinsWidget, &CoinsWidget::freeze, [&](const QVector<int>& indexes) {
        for (int i : indexes) {
            m_ctx->currentWallet->coins()->freeze(i);
        }
        m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
        m_ctx->updateBalance();
    });
    connect(ui->coinsWidget, &CoinsWidget::thaw, [&](const QVector<int>& indexes) {
        for (int i : indexes) {
            m_ctx->currentWallet->coins()->thaw(i);
        }
        m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
        m_ctx->updateBalance();
    });
    connect(ui->coinsWidget, &CoinsWidget::sweepOutput, m_ctx, &AppContext::onSweepOutput);

    connect(m_ctx, &AppContext::walletAboutToClose, [=]{
        if (!config()->get(Config::showTabHome).toBool())
            ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
        else
            ui->tabWidget->setCurrentIndex(Tabs::HOME);

        // Clear all tables when wallet is closed
        ui->historyWidget->resetModel();
        ui->contactWidget->resetModel();
        ui->receiveWidget->resetModel();
        ui->coinsWidget->resetModel();
    });

    // window title
    connect(m_ctx, &AppContext::setTitle, this, &QMainWindow::setWindowTitle);

    // init touchbar
#ifdef Q_OS_MAC
    m_touchbar = new KDMacTouchBar(this);
    m_touchbarActionWelcome = new QAction(QIcon(":/assets/images/wowlet.png"), "Welcome to WOWlet!");
    m_touchbarWalletItems = {ui->actionSettings, ui->actionCalculator, ui->actionKeys, ui->actionDonate_to_Wowlet};
    m_touchbarWizardItems = {m_touchbarActionWelcome};
#endif

    // setup some UI
    this->initMain();
    this->initWidgets();
    this->initMenu();

    connect(&m_updateBytes, &QTimer::timeout, this, &MainWindow::updateNetStats);
    ColorScheme::updateFromWidget(this);
}

void MainWindow::initMain() {
    // show wizards or open wallet directly based on cmdargs
    if(m_ctx->cmdargs->isSet("password"))
        m_ctx->walletPassword = m_ctx->cmdargs->value("password");

    QString openPath = "";
    QString autoPath = config()->get(Config::autoOpenWalletPath).toString();
    if(m_ctx->cmdargs->isSet("wallet-file"))
        openPath = m_ctx->cmdargs->value("wallet-file");
    else if(!autoPath.isEmpty())
        if (autoPath.startsWith(QString::number(m_ctx->networkType)))
            openPath = autoPath.remove(0, 1);

    if(!openPath.isEmpty() && Utils::fileExists(openPath)) {
        this->show();
        this->setEnabled(true);

        m_ctx->onOpenWallet(openPath, m_ctx->walletPassword);
        return;
    }

    this->setEnabled(false);
    this->show();
    m_wizard = this->createWizard(WalletWizard::Page_Menu);
    m_wizard->show();
    this->touchbarShowWizard();
}

void MainWindow::initMenu() {
    // setup shortcuts
    ui->actionStore_wallet->setShortcut(QKeySequence("Ctrl+S"));
    ui->actionRefresh_tabs->setShortcut(QKeySequence("Ctrl+R"));
    ui->actionClose->setShortcut(QKeySequence("Ctrl+W"));
    ui->actionShow_debug_info->setShortcut(QKeySequence("Ctrl+D"));
    ui->actionSettings->setShortcut(QKeySequence("Ctrl+Alt+S"));
    ui->actionUpdate_balance->setShortcut(QKeySequence("Ctrl+U"));

    // hide/show tabs
    m_tabShowHideSignalMapper = new QSignalMapper(this);

    connect(ui->actionShow_Home, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Home"] = new ToggleTab(ui->tabHome, "Home", "Home", ui->actionShow_Home, Config::showTabHome);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Home, "Home");

    connect(ui->actionShow_Coins, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Coins"] = new ToggleTab(ui->tabCoins, "Coins", "Coins", ui->actionShow_Coins, Config::showTabCoins);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_Coins, "Coins");

    connect(ui->actionShow_calc, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Calc"] = new ToggleTab(ui->tabCalc, "Calc", "Calc", ui->actionShow_calc, Config::showTabCalc);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_calc, "Calc");

#if defined(HAS_XMRIG)
    connect(ui->actionShow_XMRig, &QAction::triggered, m_tabShowHideSignalMapper, QOverload<>::of(&QSignalMapper::map));
    m_tabShowHideMapper["Mining"] = new ToggleTab(ui->tabXmrRig, "Mining", "Mining", ui->actionShow_XMRig, Config::showTabXMRig);
    m_tabShowHideSignalMapper->setMapping(ui->actionShow_XMRig, "Mining");
#else
    ui->actionShow_XMRig->setVisible(false);
#endif

    for (const auto &key: m_tabShowHideMapper.keys()) {
        const auto toggleTab = m_tabShowHideMapper.value(key);
        const bool show = config()->get(toggleTab->configKey).toBool();
        toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    }
    connect(m_tabShowHideSignalMapper, &QSignalMapper::mappedString, this, &MainWindow::menuToggleTabVisible);

    // Wallet
    connect(ui->actionInformation, &QAction::triggered, this, &MainWindow::showWalletInfoDialog);
    connect(ui->actionSeed, &QAction::triggered, this, &MainWindow::showSeedDialog);
    connect(ui->actionPassword, &QAction::triggered, this, &MainWindow::showPasswordDialog);
    connect(ui->actionKeys, &QAction::triggered, this, &MainWindow::showKeysDialog);
    connect(ui->actionViewOnly, &QAction::triggered, this, &MainWindow::showViewOnlyDialog);
    connect(ui->actionStore_wallet, &QAction::triggered, [this]{
        m_ctx->currentWallet->store();
    });
    connect(ui->actionRefresh_tabs, &QAction::triggered, [this]{
        m_ctx->refreshModels();
    });
    connect(ui->actionUpdate_balance, &QAction::triggered, [this]{
        m_ctx->updateBalance();
    });
    connect(ui->actionRescan_spent, &QAction::triggered, this, &MainWindow::rescanSpent);
    connect(ui->actionExportKeyImages, &QAction::triggered, this, &MainWindow::exportKeyImages);
    connect(ui->actionImportKeyImages, &QAction::triggered, this, &MainWindow::importKeyImages);
    connect(ui->actionExportOutputs, &QAction::triggered, this, &MainWindow::exportOutputs);
    connect(ui->actionImportOutputs, &QAction::triggered, this, &MainWindow::importOutputs);

    // set restore height
    connect(ui->actionChange_restore_height, &QAction::triggered, this, &MainWindow::showRestoreHeightDialog);
    connect(m_ctx, &AppContext::customRestoreHeightSet, [=](int height){
        auto msg = QString("The restore height for this wallet has been set to %1. "
                   "Please re-open the wallet. WOWlet will now quit.").arg(height);
        QMessageBox::information(this, "Cannot set custom restore height", msg);
        this->menuQuitClicked();
    });

    // CSV tx export
    connect(ui->actionExport_CSV, &QAction::triggered, [=]{
        if(m_ctx->currentWallet == nullptr) return;
        QString fn = QFileDialog::getSaveFileName(this, "Save CSV file", QDir::homePath(), "CSV (*.csv)");
        if(fn.isEmpty()) return;
        if(!fn.endsWith(".csv")) fn += ".csv";
        m_ctx->currentWallet->history()->writeCSV(fn);
        QMessageBox::information(this, "CSV export", QString("Transaction history exported to %1").arg(fn));
    });

    // Contact widget
    connect(ui->actionExportContactsCSV, &QAction::triggered, [=]{
        if(m_ctx->currentWallet == nullptr) return;
        auto *model = m_ctx->currentWallet->addressBookModel();
        if (model->rowCount() <= 0){
            QMessageBox::warning(this, "Error", "Addressbook empty");
            return;
        }

        const QString targetDir = QFileDialog::getExistingDirectory(this, "Select CSV output directory ", QDir::homePath(), QFileDialog::ShowDirsOnly);
        if(targetDir.isEmpty()) return;

        qint64 now = QDateTime::currentDateTime().currentMSecsSinceEpoch();
        QString fn = QString("%1/wownero-contacts_%2.csv").arg(targetDir, QString::number(now / 1000));
        if(model->writeCSV(fn))
            QMessageBox::information(this, "Address book exported", QString("Address book exported to %1").arg(fn));
    });

    connect(ui->actionImportContactsCSV, &QAction::triggered, this, &MainWindow::importContacts);

    // Tools
    connect(ui->actionSignVerify, &QAction::triggered, this, &MainWindow::menuSignVerifyClicked);
    connect(ui->actionVerifyTxProof, &QAction::triggered, this, &MainWindow::menuVerifyTxProof);
    connect(ui->actionLoadUnsignedTxFromFile, &QAction::triggered, this, &MainWindow::loadUnsignedTx);
    connect(ui->actionLoadUnsignedTxFromClipboard, &QAction::triggered, this, &MainWindow::loadUnsignedTxFromClipboard);
    connect(ui->actionLoadSignedTxFromFile, &QAction::triggered, this, &MainWindow::loadSignedTx);
    connect(ui->actionLoadSignedTxFromText, &QAction::triggered, this, &MainWindow::loadSignedTxFromText);
    connect(ui->actionImport_transaction, &QAction::triggered, this, &MainWindow::importTransaction);

    // About screen
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::menuAboutClicked);
    connect(ui->actionDonate_to_Wowlet, &QAction::triggered, this, &MainWindow::donateButtonClicked);

    // Close wallet
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::menuWalletCloseClicked);
}

void MainWindow::menuToggleTabVisible(const QString &key){
    const auto toggleTab = m_tabShowHideMapper[key];
    bool show = config()->get(toggleTab->configKey).toBool();
    show = !show;
    config()->set(toggleTab->configKey, show);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(toggleTab->tab), show);
    toggleTab->menuAction->setText((show ? QString("Hide ") : QString("Show ")) + toggleTab->name);
}

void MainWindow::initWidgets() {
    int homeWidget = config()->get(Config::homeWidget).toInt();
    ui->tabHomeWidget->setCurrentIndex(TabsHome(homeWidget));
}

WalletWizard *MainWindow::createWizard(WalletWizard::Page startPage){
    auto *wizard = new WalletWizard(m_ctx, startPage, this);
    connect(wizard, &WalletWizard::openWallet, m_ctx, &AppContext::onOpenWallet);
    connect(wizard, &WalletWizard::defaultWalletDirChanged, m_windowSettings, &Settings::updatePaths);
    connect(wizard, &WalletWizard::rejected, [this]{
        this->cleanupBeforeClose();
        QCoreApplication::quit();
    });
    return wizard;
}

void MainWindow::showWizard(WalletWizard::Page startPage) {
    this->setEnabled(false);
    if (m_wizard == nullptr)
        m_wizard = this->createWizard(startPage);
    m_wizard->setStartId(startPage);
    m_wizard->restart();
    m_wizard->setEnabled(true);
    m_wizard->show();
}

void MainWindow::onWalletClosed(WalletWizard::Page page) {
    m_statusLabelBalance->clear();
    m_statusLabelStatus->clear();
    this->showWizard(page);
}

void MainWindow::touchbarShowWizard() {
#ifdef Q_OS_MAC
    m_touchbar->clear();
    for(auto* action: m_touchbarWizardItems) m_touchbar->addAction(action);
#endif
}

void MainWindow::touchbarShowWallet() {
#ifdef Q_OS_MAC
    m_touchbar->clear();
    for(auto* action: m_touchbarWalletItems) m_touchbar->addAction(action);
#endif
}

void MainWindow::onWalletCreatedError(const QString &err) {
    QMessageBox::warning(this, "Wallet creation error", err);
    this->showWizard(WalletWizard::Page_CreateWallet);
}

void MainWindow::onWalletOpenPasswordRequired(bool invalidPassword, const QString &path) {
    QFileInfo fileInfo(path);

    auto dialog = new PasswordDialog(this, fileInfo.fileName(), invalidPassword);
    switch (dialog->exec()) {
        case QDialog::Rejected:
        {
            this->showWizard(WalletWizard::Page_OpenWallet);
            return;
        }
    }

    m_ctx->walletPassword = dialog->password;
    m_ctx->onOpenWallet(m_ctx->walletPath, m_ctx->walletPassword);

    dialog->deleteLater();
}

void MainWindow::onWalletOpenedError(const QString &err) {
    qDebug() << Q_FUNC_INFO << QString("Wallet open error: %1").arg(err);
    QMessageBox::warning(this, "Wallet open error", err);
    this->setWindowTitle("WOWlet");
    this->showWizard(WalletWizard::Page_OpenWallet);
    this->touchbarShowWizard();
}

void MainWindow::onWalletCreated(Wallet *wallet) {
    qDebug() << Q_FUNC_INFO;
}

void MainWindow::onWalletOpened(Wallet *wallet) {
    qDebug() << Q_FUNC_INFO;
    if(m_wizard != nullptr) {
        m_wizard->hide();
    }

    this->raise();
    this->show();
    this->activateWindow();
    this->setEnabled(true);
    if(!m_ctx->tor->torConnected)
        this->setStatusText("Wallet opened - Starting Tor (may take a while)");
    else
        this->setStatusText("Wallet opened - Searching for node");

    connect(m_ctx->currentWallet, &Wallet::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);

    // receive page
    m_ctx->currentWallet->subaddress()->refresh( m_ctx->currentWallet->currentSubaddressAccount());
    ui->receiveWidget->setModel( m_ctx->currentWallet->subaddressModel(),  m_ctx->currentWallet->subaddress());
    if (m_ctx->currentWallet->subaddress()->count() == 1) {
        for (int i = 0; i < 10; i++) {
            m_ctx->currentWallet->subaddress()->addRow(m_ctx->currentWallet->currentSubaddressAccount(), "");
        }
    }

    // history page
    m_ctx->currentWallet->history()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
    ui->historyWidget->setModel(m_ctx->currentWallet->historyModel(), m_ctx->currentWallet);

    // contacts widget
    ui->contactWidget->setModel(m_ctx->currentWallet->addressBookModel());

    // coins page
    m_ctx->currentWallet->coins()->refresh(m_ctx->currentWallet->currentSubaddressAccount());
    ui->coinsWidget->setModel(m_ctx->currentWallet->coinsModel(), m_ctx->currentWallet->coins());

    this->touchbarShowWallet();
    this->updatePasswordIcon();

    m_updateBytes.start(100);
}

void MainWindow::onBalanceUpdated(quint64 balance, quint64 spendable) {
    qDebug() << Q_FUNC_INFO;
    bool hide = config()->get(Config::hideBalance).toBool();

    QString label_str = QString("Balance: %1 WOW").arg(Utils::balanceFormat(spendable));
    if (balance > spendable)
        label_str += QString(" (+%1 WOW unconfirmed)").arg(Utils::balanceFormat(balance - spendable));

    if (hide)
        label_str = "Balance: HIDDEN";

    m_statusLabelBalance->setToolTip("Click for details");
    m_statusLabelBalance->setText(label_str);
}

void MainWindow::setStatusText(const QString &text, bool override, int timeout) {
    if (override) {
        m_statusOverrideActive = true;
        m_statusLabelStatus->setText(text);
        QTimer::singleShot(timeout, [this]{
            m_statusOverrideActive = false;
            this->setStatusText(m_statusText);
        });
        return;
    }

    m_statusText = text;

    if (!m_statusOverrideActive && !m_constructingTransaction) {
        m_statusLabelStatus->setText(text);
    }
}

void MainWindow::onSynchronized() {
    this->updateNetStats();
    this->setStatusText("Synchronized");
    this->onConnectionStatusChanged(Wallet::ConnectionStatus_Connected);
}

void MainWindow::onBlockchainSync(int height, int target) {
    QString blocks = (target >= height) ? QString::number(target - height) : "?";
    QString heightText = QString("Blockchain sync: %1 blocks remaining").arg(blocks);
    this->setStatusText(heightText);
}

void MainWindow::onRefreshSync(int height, int target) {
    QString blocks = (target >= height) ? QString::number(target - height) : "?";
    QString heightText = QString("Wallet sync: %1 blocks remaining").arg(blocks);
    this->setStatusText(heightText);
}

void MainWindow::onConnectionStatusChanged(int status)
{
    qDebug() << "Wallet connection status changed " << Utils::QtEnumToString(static_cast<Wallet::ConnectionStatus>(status));

    // Update connection info in status bar.

    QString statusIcon;
    QString statusMsg;
    switch(status){
        case Wallet::ConnectionStatus_Disconnected:
            statusIcon = ":/assets/images/status_disconnected.svg";
            this->setStatusText("Disconnected");
            break;
        case Wallet::ConnectionStatus_Connected:
            if (m_ctx->currentWallet->synchronized()) {
                statusIcon = ":/assets/images/status_connected.svg";
            } else {
                statusIcon = ":/assets/images/status_waiting.svg";
            }
            break;
        case Wallet::ConnectionStatus_Connecting:
            statusIcon = ":/assets/images/status_lagging.svg";
            this->setStatusText("Connecting to daemon");
            break;
        case Wallet::ConnectionStatus_WrongVersion:
            statusIcon = ":/assets/images/status_disconnected.svg";
            this->setStatusText("Incompatible daemon");
            break;
        default:
            statusIcon = ":/assets/images/status_disconnected.svg";
            break;
    }

    m_statusBtnConnectionStatusIndicator->setIcon(QIcon(statusIcon));
}

void MainWindow::onCreateTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address) {
    const auto &description = m_ctx->tmpTxDescription;

    // Show advanced dialog on multi-destination transactions
    if (address.size() > 1) {
        auto *dialog_adv = new TxConfAdvDialog(m_ctx, description, this);
        dialog_adv->setTransaction(tx);
        dialog_adv->exec();
        dialog_adv->deleteLater();
        return;
    }

    auto *dialog = new TxConfDialog(m_ctx, tx, address[0], description, this);
    switch (dialog->exec()) {
        case QDialog::Rejected:
        {
            if (!dialog->showAdvanced)
                m_ctx->onCancelTransaction(tx, address);
            break;
        }
        case QDialog::Accepted:
            m_ctx->currentWallet->commitTransactionAsync(tx);
            break;
    }

    if (dialog->showAdvanced) {
        auto *dialog_adv = new TxConfAdvDialog(m_ctx, description, this);
        dialog_adv->setTransaction(tx);
        dialog_adv->exec();
        dialog_adv->deleteLater();
    }
    dialog->deleteLater();
}

void MainWindow::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid) {
    if(status) { // success
        QString body = QString("Successfully sent %1 transaction(s).").arg(txid.count());
        QMessageBox::information(this, "Transactions sent", body);
        ui->sendWidget->clearFields();

        for(const auto &entry: txid) {
            m_ctx->currentWallet->setUserNote(entry, m_ctx->tmpTxDescription);
            AppContext::txDescriptionCache[entry] = m_ctx->tmpTxDescription;
        }

        m_ctx->tmpTxDescription = "";
    } else {
        auto err = tx->errorString();
        QString body = QString("Error committing transaction: %1").arg(err);
        QMessageBox::warning(this, "Transaction failed", body);
    }
}

void MainWindow::onCreateTransactionError(const QString &message) {
    auto msg = QString("Error while creating transaction: %1").arg(message);

    if (msg.contains("failed to get random outs")) {
        msg += "\n\nYour transaction has too many inputs. Try sending a lower amount.";
    }

    QMessageBox::warning(this, "Transaction failed", msg);
}

void MainWindow::create_status_bar() {
#if defined(Q_OS_WIN)
    // No seperators between statusbar widgets
    this->statusBar()->setStyleSheet("QStatusBar::item {border: None;}");
#endif

    this->statusBar()->setFixedHeight(30);

    m_statusLabelStatus = new QLabel("Idle", this);
    m_statusLabelStatus->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addWidget(m_statusLabelStatus);

    m_statusLabelNetStats = new QLabel("", this);
    m_statusLabelNetStats->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addWidget(m_statusLabelNetStats);

    m_statusLabelBalance = new ClickableLabel(this);
    m_statusLabelBalance->setText("Balance: 0 WOW");

    m_statusLabelBalance->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addPermanentWidget(m_statusLabelBalance);
    connect(m_statusLabelBalance, &ClickableLabel::clicked, this, &MainWindow::showBalanceDialog);

    m_statusBtnConnectionStatusIndicator = new StatusBarButton(QIcon(":/assets/images/status_disconnected.svg"), "Connection status");
    connect(m_statusBtnConnectionStatusIndicator, &StatusBarButton::clicked, this, &MainWindow::showConnectionStatusDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnConnectionStatusIndicator);

    m_statusBtnPassword = new StatusBarButton(QIcon(":/assets/images/lock.svg"), "Password");
    connect(m_statusBtnPassword, &StatusBarButton::clicked, this, &MainWindow::showPasswordDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnPassword);

    m_statusBtnPreferences = new StatusBarButton(QIcon(":/assets/images/preferences.svg"), "Settings");
    connect(m_statusBtnPreferences, &StatusBarButton::clicked, this, &MainWindow::menuSettingsClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnPreferences);

    m_statusBtnSeed = new StatusBarButton(QIcon(":/assets/images/seed.png"), "Seed");
    connect(m_statusBtnSeed, &StatusBarButton::clicked, this, &MainWindow::showSeedDialog);
    this->statusBar()->addPermanentWidget(m_statusBtnSeed);

    m_statusBtnTor = new StatusBarButton(QIcon(":/assets/images/tor_logo_disabled.png"), "Tor");
    connect(m_statusBtnTor, &StatusBarButton::clicked, this, &MainWindow::menuTorClicked);
    this->statusBar()->addPermanentWidget(m_statusBtnTor);
}

void MainWindow::showWalletInfoDialog() {
    auto *dialog = new WalletInfoDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showSeedDialog() {
    auto *dialog = new SeedDialog(m_ctx->currentWallet, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showConnectionStatusDialog() {
    auto status = m_ctx->currentWallet->connectionStatus();
    bool synchronized = m_ctx->currentWallet->synchronized();

    QString statusMsg;
    switch(status){
        case Wallet::ConnectionStatus_Disconnected:
            statusMsg = "Wallet is disconnected from daemon.";
            break;
        case Wallet::ConnectionStatus_Connected: {
            auto node = m_ctx->nodes->connection();
            statusMsg = QString("Wallet is connected to %1").arg(node.full);
            if (synchronized)
                statusMsg += " and synchronized";
            break;
        }
        case Wallet::ConnectionStatus_Connecting: {
            auto node = m_ctx->nodes->connection();
            statusMsg = QString("Wallet is connecting to %1").arg(node.full);
            break;
        }
        case Wallet::ConnectionStatus_WrongVersion:
            statusMsg = "Wallet is connected to incompatible daemon.";
            break;
        default:
            statusMsg = "Unknown connection status (this should never happen).";
    }

    statusMsg += QString("\n\nTx: %1, Rx: %2").arg(Utils::formatBytes(m_ctx->currentWallet->getBytesSent()),
                                                   Utils::formatBytes(m_ctx->currentWallet->getBytesReceived()));

    QMessageBox::information(this, "Connection Status", statusMsg);
}

void MainWindow::showPasswordDialog() {
    auto *pdialog = new PasswordChangeDialog(this, m_ctx->currentWallet);
    pdialog->exec();
    pdialog->deleteLater();
    this->updatePasswordIcon();
}

void MainWindow::updatePasswordIcon() {
    QIcon icon = m_ctx->currentWallet->getPassword().isEmpty() ? QIcon(":/assets/images/unlock.svg") : QIcon(":/assets/images/lock.svg");
    m_statusBtnPassword->setIcon(icon);
}

void MainWindow::showRestoreHeightDialog() {
    // settings custom restore height is only available for 25 word seeds
    auto seed = m_ctx->currentWallet->getCacheAttribute("wowlet.seed");
    if(!seed.isEmpty()) {
        const auto msg = "This wallet has a 14 word mnemonic seed which has the restore height embedded.";
        QMessageBox::warning(this, "Cannot set custom restore height", msg);
        return;
    }

    m_restoreDialog = new RestoreDialog(m_ctx, this);
    m_restoreDialog->show();

    connect(m_restoreDialog, &RestoreDialog::accepted, [=]{
        auto height = m_restoreDialog->getHeight();
        m_restoreDialog->disconnect();
        m_restoreDialog->deleteLater();
        m_ctx->onSetRestoreHeight(height);
    });

    connect(m_restoreDialog, &RestoreDialog::rejected, [=]{
        m_restoreDialog->disconnect();
        m_restoreDialog->deleteLater();
    });
}

void MainWindow::showKeysDialog() {
    auto *dialog = new KeysDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showViewOnlyDialog() {
    auto *dialog = new ViewOnlyDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::menuTorClicked() {

    auto *dialog = new TorInfoDialog(m_ctx, this);
    connect(m_ctx->tor, &Tor::logsUpdated, dialog, &TorInfoDialog::onLogsUpdated);
    dialog->exec();
    disconnect(m_ctx->tor, &Tor::logsUpdated, dialog, &TorInfoDialog::onLogsUpdated);
    dialog->deleteLater();
}

void MainWindow::menuNewRestoreClicked() {
    // TODO: implement later
}

void MainWindow::menuQuitClicked() {
    cleanupBeforeClose();

    QCoreApplication::quit();
}

void MainWindow::menuWalletCloseClicked() {
    if (m_ctx->currentWallet == nullptr)
        return;

    m_ctx->closeWallet(true, true);
}

void MainWindow::menuWalletOpenClicked() {
    auto walletPath = config()->get(Config::walletPath).toString();
    if(walletPath.isEmpty())
        walletPath = m_ctx->defaultWalletDir;
    QString path = QFileDialog::getOpenFileName(this, "Select your wallet file", walletPath, "Wallet file (*.keys)");
    if(path.isEmpty()) return;

    QFileInfo infoPath(path);
    if(!infoPath.isReadable()) {
        QMessageBox::warning(this, "Cannot read wallet file", "Permission error.");
        return;
    }

    if(path == m_ctx->walletPath) {
        QMessageBox::warning(this, "Wallet already opened", "Please open another wallet.");
        return;
    }

    m_ctx->closeWallet(false);
    m_ctx->onOpenWallet(path, "");
}

void MainWindow::menuAboutClicked() {
    m_aboutDialog->show();
}

void MainWindow::menuSettingsClicked() {
    m_windowSettings->raise();
    m_windowSettings->show();
    m_windowSettings->activateWindow();
}

void MainWindow::menuSignVerifyClicked() {
    auto *dialog = new SignVerifyDialog(m_ctx->currentWallet, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::menuVerifyTxProof() {
    auto *dialog = new VerifyProofDialog(m_ctx->currentWallet, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::skinChanged(const QString &skinName) {
    if(!m_skins.contains(skinName)) {
        qWarning() << QString("No such skin %1").arg(skinName);
        return;
    }

    config()->set(Config::skin, skinName);
    qApp->setStyleSheet(m_skins[skinName]);
    qDebug() << QString("Skin changed to %1").arg(skinName);
    ColorScheme::updateFromWidget(this);

    ui->conversionWidget->skinChanged();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    cleanupBeforeClose();

    QWidget::closeEvent(event);
}

void MainWindow::donateButtonClicked() {
    double donation = AppContext::prices->convert("EUR", "WOW", m_ctx->donationAmount);
    if (donation <= 0)
        donation = 0.1337;

    ui->sendWidget->fill(m_ctx->donationAddress, "Donation to the WOWlet development team", donation);
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::showHistoryTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(Tabs::HISTORY);
}

void MainWindow::showSendTab() {
    this->raise();
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::showCalcWindow() {
    m_windowCalc->show();
}

void MainWindow::payToMany() {
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
    ui->sendWidget->payToMany();
    QMessageBox::information(this, "Pay to many", "Enter a list of outputs in the 'Pay to' field.\n"
                                                  "One output per line.\n"
                                                  "Format: address, amount\n"
                                                  "A maximum of 16 addresses may be specified.");
}

void MainWindow::showSendScreen(const CCSEntry &entry) {
    ui->sendWidget->fill(entry);
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::suchDonate(const QString address) {
    double donation = AppContext::prices->convert("EUR", "WOW", 2);
    ui->sendWidget->fill(address, "SuchWow contribution :-)", donation);
    ui->tabWidget->setCurrentIndex(Tabs::SEND);
}

void MainWindow::onViewOnBlockExplorer(const QString &txid) {
    QString blockExplorerLink = Utils::blockExplorerLink(config()->get(Config::blockExplorer).toString(), m_ctx->networkType, txid);
    Utils::externalLinkWarning(this, blockExplorerLink);
}

void MainWindow::onResendTransaction(const QString &txid) {
    if (!AppContext::txCache.contains(txid)) {
        QMessageBox::warning(this, "Unable to resend transaction", "Transaction was not found in transaction cache. Unable to resend.");
        return;
    }

    // Connect to a different node so chances of successful relay are higher
    m_ctx->nodes->autoConnect(true);

    auto dialog = new BroadcastTxDialog(this, m_ctx, AppContext::txCache[txid]);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::importContacts() {
    const QString targetFile = QFileDialog::getOpenFileName(this, "Import CSV file", QDir::homePath(), "CSV Files (*.csv)");
    if(targetFile.isEmpty()) return;

    auto *model = m_ctx->currentWallet->addressBookModel();
    QMapIterator<QString, QString> i(model->readCSV(targetFile));
    int inserts = 0;
    while (i.hasNext()) {
        i.next();
        bool addressValid = WalletManager::addressValid(i.value(), m_ctx->currentWallet->nettype());
        if(addressValid) {
            m_ctx->currentWallet->addressBook()->addRow(i.value(), "", i.key());
            inserts++;
        }
    }

    QMessageBox::information(this, "Contacts imported", QString("Total contacts imported: %1").arg(inserts));
}

MainWindow *MainWindow::getInstance() {
    return pMainWindow;
}

AppContext *MainWindow::getContext(){
    return pMainWindow->m_ctx;
}

void MainWindow::loadSkins() {
    QString breeze_wow = this->loadStylesheet(":/wow.qss");
    if (!breeze_wow.isEmpty())
        m_skins.insert("WOW", breeze_wow);
    
    QString qdarkstyle = this->loadStylesheet(":qdarkstyle/style.qss");
    if (!qdarkstyle.isEmpty())
        m_skins.insert("QDarkStyle", qdarkstyle);

    QString breeze_dark = this->loadStylesheet(":/dark.qss");
    if (!breeze_dark.isEmpty())
        m_skins.insert("Breeze/Dark", breeze_dark);

    QString breeze_light = this->loadStylesheet(":/light.qss");
    if (!breeze_light.isEmpty())
        m_skins.insert("Breeze/Light", breeze_light);
}

QString MainWindow::loadStylesheet(const QString &resource) {
    QFile f(resource);
    if (!f.exists()) {
        printf("Unable to set stylesheet, file not found\n");
        f.close();
        return "";
    }

    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    QString data = ts.readAll();
    f.close();

    return data;
}

void MainWindow::saveGeo() {
    config()->set(Config::geometry, QString(saveGeometry().toBase64()));
    config()->set(Config::windowState, QString(saveState().toBase64()));
}

void MainWindow::restoreGeo() {
    bool geo = this->restoreGeometry(QByteArray::fromBase64(config()->get(Config::geometry).toByteArray()));
    bool windowState = this->restoreState(QByteArray::fromBase64(config()->get(Config::windowState).toByteArray()));
    qDebug() << "Restored window state: " << geo << " " << windowState;
}

void MainWindow::showDebugInfo() {
    auto *dialog = new DebugInfoDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showWalletCacheDebugDialog() {
    auto *dialog = new WalletCacheDebugDialog(m_ctx, this);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::showNodeExhaustedMessage() {
    // Spawning dialogs inside a lambda can cause system freezes on linux so we have to do it this way ¯\_(ツ)_/¯

    auto msg = "WOWlet is in 'custom node connection mode' but could not "
               "find an eligible node to connect to. Please go to Settings->Node "
               "and enter a node manually.";
    QMessageBox::warning(this, "Could not connect to a node", msg);
}

void MainWindow::showWSNodeExhaustedMessage() {
    auto msg = "WOWlet is in 'automatic node connection mode' but the "
               "websocket server returned no available nodes. Please go to Settings->Node "
               "and enter a node manually.";
    QMessageBox::warning(this, "Could not connect to a node", msg);
}

void MainWindow::exportKeyImages() {
    QString fn = QFileDialog::getSaveFileName(this, "Save key images to file", QDir::homePath(), "Key Images (*_keyImages)");
    if (fn.isEmpty()) return;
    if (!fn.endsWith("_keyImages")) fn += "_keyImages";
    m_ctx->currentWallet->exportKeyImages(fn, true);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Key image export", QString("Failed to export key images.\nReason: %1").arg(err));
    } else {
        QMessageBox::information(this, "Key image export", "Successfully exported key images.");
    }
}

void MainWindow::importKeyImages() {
    QString fn = QFileDialog::getOpenFileName(this, "Import key image file", QDir::homePath(), "Key Images (*_keyImages)");
    if (fn.isEmpty()) return;
    m_ctx->currentWallet->importKeyImages(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Key image import", QString("Failed to import key images.\n\n%1").arg(err));
    } else {
        QMessageBox::information(this, "Key image import", "Successfully imported key images");
        m_ctx->refreshModels();
    }
}

void MainWindow::exportOutputs() {
    QString fn = QFileDialog::getSaveFileName(this, "Save outputs to file", QDir::homePath(), "Outputs (*_outputs)");
    if (fn.isEmpty()) return;
    if (!fn.endsWith("_outputs")) fn += "_outputs";
    m_ctx->currentWallet->exportOutputs(fn, true);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Outputs export", QString("Failed to export outputs.\nReason: %1").arg(err));
    } else {
        QMessageBox::information(this, "Outputs export", "Successfully exported outputs.");
    }
}

void MainWindow::importOutputs() {
    QString fn = QFileDialog::getOpenFileName(this, "Import outputs file", QDir::homePath(), "Outputs (*_outputs)");
    if (fn.isEmpty()) return;
    m_ctx->currentWallet->importOutputs(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Outputs import", QString("Failed to import outputs.\n\n%1").arg(err));
    } else {
        QMessageBox::information(this, "Outputs import", "Successfully imported outputs");
        m_ctx->refreshModels();
    }
}

void MainWindow::cleanupBeforeClose() {
    m_ctx->closeWallet(false, true);
    m_ctx->tor->stop();

    this->saveGeo();
}

void MainWindow::loadUnsignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*unsigned_monero_tx)");
    if (fn.isEmpty()) return;
    UnsignedTransaction *tx = m_ctx->currentWallet->loadTxFile(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load transaction from file", QString("Failed to load transaction.\n\n%1").arg(err));
        return;
    }

    this->createUnsignedTxDialog(tx);
}

void MainWindow::loadUnsignedTxFromClipboard() {
    QString unsigned_tx = Utils::copyFromClipboard();
    if (unsigned_tx.isEmpty()) {
        QMessageBox::warning(this, "Load unsigned transaction from clipboard", "Clipboard is empty");
        return;
    }
    UnsignedTransaction *tx = m_ctx->currentWallet->loadTxFromBase64Str(unsigned_tx);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load unsigned transaction from clipboard", QString("Failed to load transaction.\n\n%1").arg(err));
        return;
    }

    this->createUnsignedTxDialog(tx);
}

void MainWindow::loadSignedTx() {
    QString fn = QFileDialog::getOpenFileName(this, "Select transaction to load", QDir::homePath(), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) return;
    PendingTransaction *tx = m_ctx->currentWallet->loadSignedTxFile(fn);
    auto err = m_ctx->currentWallet->errorString();
    if (!err.isEmpty()) {
        QMessageBox::warning(this, "Load signed transaction from file", err);
        return;
    }

    auto *dialog = new TxConfAdvDialog(m_ctx, "", this);
    dialog->setTransaction(tx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::loadSignedTxFromText() {
    auto dialog = new BroadcastTxDialog(this, m_ctx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::createUnsignedTxDialog(UnsignedTransaction *tx) {
    auto *dialog = new TxConfAdvDialog(m_ctx, "", this);
    dialog->setUnsignedTransaction(tx);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::importTransaction() {

    auto result = QMessageBox::warning(this, "Warning", "Using this feature may allow a remote node to associate the transaction with your IP address.\n"
                                          "\n"
                                          "Connect to a trusted node or run WOWlet over Tor if network level metadata leakage is included in your threat model.",
                                          QMessageBox::Ok | QMessageBox::Cancel);
    if (result == QMessageBox::Ok) {
        auto *dialog = new TxImportDialog(this, m_ctx);
        dialog->exec();
        dialog->deleteLater();
    }
}

void MainWindow::updateNetStats() {
    if (m_ctx->currentWallet == nullptr) {
        m_statusLabelNetStats->setText("");
        return;
    }

    if (m_ctx->currentWallet->connectionStatus() == Wallet::ConnectionStatus_Connected && m_ctx->currentWallet->synchronized()) {
        m_statusLabelNetStats->setText("");
        return;
    }

    m_statusLabelNetStats->setText(QString("(D: %1)").arg(Utils::formatBytes(m_ctx->currentWallet->getBytesReceived())));
}

void MainWindow::rescanSpent() {
    if (!m_ctx->currentWallet->rescanSpent()) {
        QMessageBox::warning(this, "Rescan spent", m_ctx->currentWallet->errorString());
    } else {
        QMessageBox::information(this, "Rescan spent", "Successfully rescanned spent outputs.");
    }
}

void MainWindow::showBalanceDialog() {
    if (!m_ctx->currentWallet) {
        return;
    }
    auto *dialog = new BalanceDialog(this, m_ctx->currentWallet);
    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::onUpdateSATWidget() {
    if(!AppContext::prices->markets.count() || !AppContext::prices->rates.count())
        return;
    auto wowObj = AppContext::prices->markets["WOW"];
    QString satStr = QString::number(wowObj.price_sat);
    m_tickerSAT->setFiatText(satStr);
}

void MainWindow::onUpdateWowWidget() {
    if(!AppContext::prices->markets.count() || !AppContext::prices->rates.count())
        return;
    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if(!AppContext::prices->rates.contains(fiatCurrency)){
        config()->set(Config::preferredFiatCurrency, "USD");
        fiatCurrency = "USD";
    }

    auto wowObj = AppContext::prices->markets["WOW"];
    auto currencyText = Utils::amountToCurrencyString(wowObj.price_usd, fiatCurrency);
    m_tickerWOW->setFiatText(currencyText);

    auto pct24h = AppContext::prices->markets["WOW"].price_usd_change_pct_24h;
    auto pct24hText = QString::number(pct24h, 'f', 2);
    this->m_tickerWOW->setPctText(pct24hText, pct24h >= 0.0);
}

void MainWindow::onUpdateBTCWidget() {
    if(!AppContext::prices->markets.count() || !AppContext::prices->rates.count())
        return;
    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if(!AppContext::prices->rates.contains(fiatCurrency)){
        config()->set(Config::preferredFiatCurrency, "USD");
        fiatCurrency = "USD";
    }

    double conversion = AppContext::prices->convert("BTC", fiatCurrency, 1.0);
    auto conversionText = Utils::amountToCurrencyString(conversion, fiatCurrency);
    m_tickerBTC->setFiatText(conversionText);

    auto pct24h = AppContext::prices->markets["BTC"].price_usd_change_pct_24h;
    auto pct24hText = QString::number(pct24h, 'f', 2);
    this->m_tickerBTC->setPctText(pct24hText, pct24h >= 0.0);
}

void MainWindow::onUpdateXMRWidget() {
    if(!AppContext::prices->markets.count() || !AppContext::prices->rates.count())
        return;
    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if(!AppContext::prices->rates.contains(fiatCurrency)){
        config()->set(Config::preferredFiatCurrency, "USD");
        fiatCurrency = "USD";
    }

    double conversion = AppContext::prices->convert("XMR", fiatCurrency, 1.0);
    auto conversionText = Utils::amountToCurrencyString(conversion, fiatCurrency);
    m_tickerXMR->setFiatText(conversionText);

    auto pct24h = AppContext::prices->markets["XMR"].price_usd_change_pct_24h;
    auto pct24hText = QString::number(pct24h, 'f', 2);
    this->m_tickerXMR->setPctText(pct24hText, pct24h >= 0.0);
}

void MainWindow::onUpdateFiatBalanceWidget() {
    bool hide = config()->get(Config::hideBalance).toBool();
    m_balanceWidget->setHidden(hide);

    if(!AppContext::prices->markets.count() || !AppContext::prices->rates.count())
        return;
    QString fiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if(!AppContext::prices->rates.contains(fiatCurrency)){
        config()->set(Config::preferredFiatCurrency, "USD");
        fiatCurrency = "USD";
    }

    double conversion = AppContext::prices->convert("WOW", fiatCurrency, AppContext::balance);
    auto conversionText = Utils::amountToCurrencyString(conversion, fiatCurrency);
    m_balanceWidget->setFiatText(conversionText);
}

QString MainWindow::statusDots() {
    m_statusDots++;
    m_statusDots = m_statusDots % 4;
    return QString(".").repeated(m_statusDots);
}

MainWindow::~MainWindow() {
    delete ui;
}
