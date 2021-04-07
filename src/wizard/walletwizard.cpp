// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "utils/utils.h"
#include "wizard/walletwizard.h"
#include "wizard/menu.h"
#include "wizard/openwallet.h"
#include "wizard/createwallet.h"
#include "wizard/network.h"
#include "wizard/createwalletseed.h"
#include "wizard/restorewallet.h"
#include "wizard/viewonlywallet.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QScreen>

WalletWizard::WalletWizard(AppContext *ctx, WalletWizard::Page startPage, QWidget *parent)
        : QWizard(parent)
        , m_ctx(ctx)
{
    this->setWindowTitle("Welcome to WOWlet");
    this->setWindowIcon(QIcon(":/assets/images/appicons/64x64.png"));

    m_walletKeysFilesModel = new WalletKeysFilesModel(m_ctx, this);
    m_walletKeysFilesModel->refresh();

    auto openWalletPage = new OpenWalletPage(m_ctx, m_walletKeysFilesModel, this);
    auto createWallet = new CreateWalletPage(m_ctx, this);
    auto createWalletSeed = new CreateWalletSeedPage(m_ctx, this);
    setPage(Page_Menu, new MenuPage(m_ctx, m_walletKeysFilesModel, this));
    setPage(Page_CreateWallet, createWallet);
    setPage(Page_OpenWallet, openWalletPage);
    setPage(Page_CreateWalletSeed, createWalletSeed);
    setPage(Page_Network, new NetworkPage(m_ctx, this));
    setPage(Page_Restore, new RestorePage(m_ctx, this));
    setPage(Page_ViewOnly, new ViewOnlyPage(m_ctx, this));

    if(config()->get(Config::firstRun).toUInt())
        setStartId(Page_Network);
    else
        setStartId(Page_Menu);

    setButtonText(QWizard::CancelButton, "Close");
    QStringList headers = {
            ":/assets/images/welcome/wow1.png",
            ":/assets/images/welcome/wow2.png",
            ":/assets/images/welcome/wow3.png",
            ":/assets/images/welcome/wow4.png",
            ":/assets/images/welcome/wow5.png",
            ":/assets/images/welcome/wow6.png",
            ":/assets/images/welcome/wow7.png",
            ":/assets/images/welcome/wow8.png",
            ":/assets/images/welcome/wow9.png",
            ":/assets/images/welcome/wow10.png",
            ":/assets/images/welcome/wow11.png",
            ":/assets/images/welcome/wow12.png",
            ":/assets/images/welcome/wow13.png",
            ":/assets/images/welcome/wow14.png",
            ":/assets/images/welcome/wow15.png",
            ":/assets/images/welcome/wow16.png",
            ":/assets/images/welcome/wow17.png",
            ":/assets/images/welcome/wow18.png",
            ":/assets/images/welcome/wow19.png",
            ":/assets/images/welcome/wow20.png",
            ":/assets/images/welcome/wow21.png",
            ":/assets/images/welcome/wow22.png",
            ":/assets/images/welcome/wow23.png",
            ":/assets/images/welcome/wow24.png",
            ":/assets/images/welcome/wow25.png",
            ":/assets/images/welcome/wow26.png",
            ":/assets/images/welcome/wow27.png",
            ":/assets/images/welcome/wow28.png",
            ":/assets/images/welcome/wow29.png",
            ":/assets/images/welcome/wow30.png",
            ":/assets/images/welcome/wow31.png",
            ":/assets/images/welcome/wow32.png",
            ":/assets/images/welcome/wow33.png",
            ":/assets/images/welcome/wow34.png",
            ":/assets/images/welcome/wow35.png",
            ":/assets/images/welcome/wow36.png",
            ":/assets/images/welcome/wow37.png",
            ":/assets/images/welcome/wow38.png",
            ":/assets/images/welcome/wow39.png",
            ":/assets/images/welcome/wow40.png",
            ":/assets/images/welcome/wow41.png",
            ":/assets/images/welcome/wow42.png",
            ":/assets/images/welcome/wow43.png",
            ":/assets/images/welcome/wow44.png",
            ":/assets/images/welcome/wow45.png",
            ":/assets/images/welcome/wow46.png",
            ":/assets/images/welcome/wow47.png"
    };
    int random = QRandomGenerator::global()->bounded(headers.count());
    auto header = headers.at(random);
    setPixmap(QWizard::WatermarkPixmap, QPixmap(header));

    setWizardStyle(WizardStyle::ModernStyle);
    setOption(QWizard::NoBackButtonOnStartPage);

    connect(this, &QWizard::rejected, [=]{
        return QApplication::exit(1);
    });

    connect(createWalletSeed, &CreateWalletSeedPage::createWallet, this, &WalletWizard::createWallet);
    connect(createWallet, &CreateWalletPage::createWallet, this, &WalletWizard::createWallet);
    connect(createWallet, &CreateWalletPage::defaultWalletDirChanged, [this](const QString &walletDir){
        emit defaultWalletDirChanged(walletDir);
    });

    connect(openWalletPage, &OpenWalletPage::openWallet, [=](const QString &path){
        const auto walletPasswd = this->field("walletPasswd").toString();
        emit openWallet(path, walletPasswd);
    });
}

void WalletWizard::createWallet() {
    auto mnemonicRestoredSeed = this->field("mnemonicRestoredSeed").toString();
    auto mnemonicSeed = mnemonicRestoredSeed.isEmpty() ? this->field("mnemonicSeed").toString() : mnemonicRestoredSeed;
    const auto walletPath = this->field("walletPath").toString();
    const auto walletPasswd = this->field("walletPasswd").toString();
    auto restoreHeight = this->field("restoreHeight").toUInt();
    auto viewKey = this->field("viewOnlyViewKey").toString();
    auto spendKey = this->field("viewOnlySpendKey").toString();
    auto viewAddress = this->field("viewOnlyAddress").toString();

    if(!viewKey.isEmpty() && !viewAddress.isEmpty()) {
        auto viewHeight = this->field("viewOnlyHeight").toUInt();
        m_ctx->createWalletViewOnly(walletPath,
                                    walletPasswd,
                                    viewAddress,
                                    viewKey, spendKey, viewHeight);
        return;
    }

    auto seed = WowletSeed(m_ctx->restoreHeights[m_ctx->networkType], m_ctx->coinName, m_ctx->seedLanguage, mnemonicSeed.split(" "));

    if(restoreHeight > 0)
        seed.setRestoreHeight(restoreHeight);
    m_ctx->createWallet(seed, walletPath, walletPasswd);
}
