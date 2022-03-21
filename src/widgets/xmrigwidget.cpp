// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QStandardItemModel>
#include <QTableWidget>
#include <QMessageBox>
#include <QDesktopServices>
#include <QScrollBar>
#include <QFileDialog>

#include "xmrigwidget.h"
#include "ui_xmrigwidget.h"

XMRigWidget::XMRigWidget(AppContext *ctx, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::XMRigWidget),
    m_ctx(ctx),
    m_modelRig(new QStandardItemModel(this)),
    m_modelWownerod(new QStandardItemModel(this)),
    m_contextMenuRig(new QMenu(this)),
    m_contextMenuWownerod(new QMenu(this))
{
    ui->setupUi(this);
    this->resetUI();

    QPixmap p(":assets/images/fire.png");
    ui->lbl_logo->setPixmap(p.scaled(268, 271, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // table XMRig
    ui->tableRig->setModel(this->m_modelRig);
    m_contextMenuRig->addAction(QIcon(":/assets/images/network.png"), "Download file", this, &XMRigWidget::rigLinkClicked);
    connect(ui->tableRig, &QHeaderView::customContextMenuRequested, this, &XMRigWidget::showContextRigMenu);
    connect(ui->tableRig, &QTableView::doubleClicked, this, &XMRigWidget::rigLinkClicked);

    // table wownerod
    ui->tableWownerod->setModel(this->m_modelWownerod);
    m_contextMenuWownerod->addAction(QIcon(":/assets/images/network.png"), "Download file", this, &XMRigWidget::wownerodLinkClicked);
    connect(ui->tableWownerod, &QHeaderView::customContextMenuRequested, this, &XMRigWidget::showContextWownerodMenu);
    connect(ui->tableWownerod, &QTableView::doubleClicked, this, &XMRigWidget::wownerodLinkClicked);

    // threads
    ui->threadSlider->setMinimum(1);
    int threads = QThread::idealThreadCount();
    m_threads = threads / 2;
    ui->threadSlider->setMaximum(threads);
    ui->threadSlider->setValue(m_threads);
    ui->label_threads->setText(QString("CPU threads: %1").arg(m_threads));
    connect(ui->threadSlider, &QSlider::valueChanged, this, &XMRigWidget::onThreadsValueChanged);

    // buttons
    connect(ui->btn_start, &QPushButton::clicked, this, &XMRigWidget::onStartClicked);
    connect(ui->btn_stop, &QPushButton::clicked, this, &XMRigWidget::onStopClicked);
    connect(ui->btn_browse, &QPushButton::clicked, this, &XMRigWidget::onBrowseClicked);
    connect(ui->btn_clear, &QPushButton::clicked, this, &XMRigWidget::onClearClicked);

    // graphics
    bool simplifiedUI = config()->get(Config::simplifiedMiningInterface).toBool();
    ui->comboBox_gfx->setCurrentIndex(simplifiedUI ? 1 : 0);
    connect(ui->comboBox_gfx, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &XMRigWidget::onSimplifiedMiningChanged);

    // wownerod binary
    auto path = config()->get(Config::wownerodPath).toString();
    if(!path.isEmpty())
        ui->lineEdit_path->setText(path);

    connect(ui->lineEdit_path, &QLineEdit::textChanged, [=] {
      config()->set(Config::wownerodPath, ui->lineEdit_path->text().trimmed());
    });

    // info
    this->appendText(QString("Detected %1 CPU threads.").arg(threads));
    if(path.isEmpty())
        this->appendText(QString("wownerod path is empty - please point towards the wownerod executable.").arg(path));
    else if(!Utils::fileExists(path))
        this->appendText("Invalid path to the wownerod executable detected. Please set the correct path.");
    else {
        this->appendText(QString("wownerod path set to '%1'").arg(path));
        this->appendText("Ready to mine.");
    }
}

void XMRigWidget::resetUI() {
    ui->consoleFrame->hide();
    ui->qmlFrame->hide();
    ui->qmlFrameTxt->hide();

    ui->check_autoscroll->setChecked(true);
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_status->hide();
    ui->console->clear();

    this->destroyQml();
}

void XMRigWidget::startUI() {
    this->resetUI();
    bool simplifiedUI = config()->get(Config::simplifiedMiningInterface).toBool();

    if(simplifiedUI) {
        this->initConsole();
    } else {
        this->initQML();
    }
}

void XMRigWidget::initConsole() {
    ui->consoleFrame->show();
}

void XMRigWidget::initQML() {
    if(m_quickWidget != nullptr) return;
    m_quickWidget = new QQuickWidget(this);

    auto *qctx = m_quickWidget->rootContext();
    qctx->setContextProperty("cfg", config());
    qctx->setContextProperty("ctx", m_ctx);
    qctx->setContextProperty("mining", this);

    m_quickWidget->setSource(QUrl("qrc:/mining.qml"));
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    connect((QObject*)m_quickWidget->rootObject(), SIGNAL(startMining()),
            this, SLOT(onStartClicked()));

    connect((QObject*)m_quickWidget->rootObject(), SIGNAL(stopMining()),
            this, SLOT(onStopClicked()));

    ui->qmlFrame->layout()->addWidget(m_quickWidget);
    ui->qmlFrame->show();
    qDebug() << "created QML mining widget";
}

void XMRigWidget::destroyQml() {
    if(m_quickWidget == nullptr) return;
    m_quickWidget->disconnect();
    m_quickWidget->deleteLater();
    m_quickWidget = nullptr;
    qDebug() << "destroyed QML mining widget";
}

void XMRigWidget::appendText(const QString &line) {
    ui->console->appendPlainText(line);
    m_consoleBuffer += 1;
    if(m_consoleBuffer >= m_consoleBufferMax) {
        ui->console->clear();
        m_consoleBuffer = 0;
    }
}

void XMRigWidget::onWalletClosed() {
    this->onStopClicked();
    this->onClearClicked();
}

void XMRigWidget::onWalletOpened(Wallet *wallet){
    int egiwoge = 1;
}

void XMRigWidget::onThreadsValueChanged(int threads) {
    m_threads = threads;
    ui->label_threads->setText(QString("CPU threads: %1").arg(m_threads));
}

void XMRigWidget::onBrowseClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Path to wownerod executable", QDir::homePath());
    if (fileName.isEmpty()) return;
    config()->set(Config::wownerodPath, fileName);
    ui->lineEdit_path->setText(fileName);
}

void XMRigWidget::onSyncStatus(unsigned int from, unsigned int to, unsigned int pct) {
    emit syncStatus(from, to, pct);
}

void XMRigWidget::onDaemonStateChanged(DaemonMiningState state) {
    if(state == DaemonMiningState::idle) {
        ui->btn_stop->setEnabled(false);
        ui->btn_start->setEnabled(true);
        ui->label_status->hide();
    } else {
        ui->btn_stop->setEnabled(true);
        ui->btn_start->setEnabled(false);
        ui->label_status->show();
    }

    m_daemonMiningState = state;
    emit daemonMiningStateChanged();
}

void XMRigWidget::onUptimeChanged(const QString &uptime) {
    emit uptimeChanged(uptime);
}

void XMRigWidget::onBlockReward() {
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("yyyy/MM/dd hh:mm");

    auto reward = QString("Congrats: new block found at %1").arg(formattedTime);

    // @TODO: this might be blocking, what if multiple rewards happen?
    QMessageBox::information(this, "Reward found", reward);
}

void XMRigWidget::onClearClicked() {
    ui->console->clear();
}

void XMRigWidget::onStartClicked() {
    auto binPath = config()->get(Config::wownerodPath).toString();
    if(!m_ctx->XMRig->start(binPath, m_threads)) return;

    ui->btn_start->setEnabled(false);
    ui->btn_stop->setEnabled(true);
}

void XMRigWidget::onStopClicked() {
    if(m_ctx->XMRig->daemonMiningState != DaemonMiningState::idle)
        m_ctx->XMRig->stop();
}

void XMRigWidget::onProcessOutput(const QByteArray &data) {
    auto line = Utils::barrayToString(data);
    line = line.trimmed();
    this->appendText(line);

    if(ui->check_autoscroll->isChecked())
        ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
}

void XMRigWidget::onProcessError(const QString &msg) {
    this->appendText(msg);
}

void XMRigWidget::onSimplifiedMiningChanged(int idx) {
    config()->set(Config::simplifiedMiningInterface, idx == 1);
    this->startUI();
}

void XMRigWidget::onHashrate(const QString &rate) {
    ui->label_status->show();
    ui->label_status->setText(QString("Mining at %1").arg(rate));
    emit hashrate(rate);
}

void XMRigWidget::onWownerodDownloads(const QJsonObject &data) {
    m_modelWownerod->clear();
    m_urlsWownerod.clear();

    auto version = data.value("version").toString();
    ui->label_latest_version_wownerod->setText(QString("Latest version: %1").arg(version));
    QJsonObject assets = data.value("assets").toObject();

    const auto _linux = assets.value("linux").toArray();
    const auto macos = assets.value("macos").toArray();
    const auto windows = assets.value("windows").toArray();

    auto info = QSysInfo::productType();
    QJsonArray *os_assets;
    if(info == "osx") {
        os_assets = const_cast<QJsonArray *>(&macos);
    } else if (info == "windows") {
        os_assets = const_cast<QJsonArray *>(&windows);
    } else {
        // assume linux
        os_assets = const_cast<QJsonArray *>(&_linux);
    }

    int i = 0;
    for(const auto &entry: *os_assets) {
        auto _obj = entry.toObject();
        auto _name = _obj.value("name").toString();
        auto _url = _obj.value("url").toString();
        auto _created_at = _obj.value("created_at").toString();

        m_urlsWownerod.append(_url);
        auto download_count = _obj.value("download_count").toInt();

        m_modelWownerod->setItem(i, 0, Utils::qStandardItem(_name));
        m_modelWownerod->setItem(i, 1, Utils::qStandardItem(_created_at));
        m_modelWownerod->setItem(i, 2, Utils::qStandardItem(QString::number(download_count)));
        i++;
    }

    m_modelWownerod->setHeaderData(0, Qt::Horizontal, tr("Filename"), Qt::DisplayRole);
    m_modelWownerod->setHeaderData(1, Qt::Horizontal, tr("Date"), Qt::DisplayRole);
    m_modelWownerod->setHeaderData(2, Qt::Horizontal, tr("Downloads"), Qt::DisplayRole);

    ui->tableWownerod->verticalHeader()->setVisible(false);
    ui->tableWownerod->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWownerod->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWownerod->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWownerod->setColumnWidth(2, 100);
}

void XMRigWidget::onMenuTabChanged(int index) {
    if(m_tabIndex == globals::Tabs::XMRIG && index != m_tabIndex)
        this->resetUI();
    else if(globals::Tabs(index + 1) == globals::Tabs::XMRIG)
        this->startUI();
    m_tabIndex = index + 1;
}

void XMRigWidget::onRigDownloads(const QJsonObject &data) {
    m_modelRig->clear();
    m_urlsRig.clear();

    auto version = data.value("version").toString();
    ui->label_latest_version_rig->setText(QString("Latest version: %1").arg(version));
    QJsonObject assets = data.value("assets").toObject();

    const auto _linux = assets.value("linux").toArray();
    const auto macos = assets.value("macos").toArray();
    const auto windows = assets.value("windows").toArray();

    auto info = QSysInfo::productType();
    QJsonArray *os_assets;
    if(info == "osx") {
        os_assets = const_cast<QJsonArray *>(&macos);
    } else if (info == "windows") {
        os_assets = const_cast<QJsonArray *>(&windows);
    } else {
        // assume linux
        os_assets = const_cast<QJsonArray *>(&_linux);
    }

    int i = 0;
    for(const auto &entry: *os_assets) {
        auto _obj = entry.toObject();
        auto _name = _obj.value("name").toString();
        auto _url = _obj.value("url").toString();
        auto _created_at = _obj.value("created_at").toString();

        m_urlsRig.append(_url);
        auto download_count = _obj.value("download_count").toInt();

        m_modelRig->setItem(i, 0, Utils::qStandardItem(_name));
        m_modelRig->setItem(i, 1, Utils::qStandardItem(_created_at));
        m_modelRig->setItem(i, 2, Utils::qStandardItem(QString::number(download_count)));
        i++;
    }

    m_modelRig->setHeaderData(0, Qt::Horizontal, tr("Filename"), Qt::DisplayRole);
    m_modelRig->setHeaderData(1, Qt::Horizontal, tr("Date"), Qt::DisplayRole);
    m_modelRig->setHeaderData(2, Qt::Horizontal, tr("Downloads"), Qt::DisplayRole);

    ui->tableRig->verticalHeader()->setVisible(false);
    ui->tableRig->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableRig->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableRig->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableRig->setColumnWidth(2, 100);
}

void XMRigWidget::showContextRigMenu(const QPoint &pos) {
    QModelIndex index = ui->tableRig->indexAt(pos);
    if (!index.isValid())
        return;
    m_contextMenuRig->exec(ui->tableRig->viewport()->mapToGlobal(pos));
}

void XMRigWidget::showContextWownerodMenu(const QPoint &pos) {
    QModelIndex index = ui->tableWownerod->indexAt(pos);
    if (!index.isValid())
        return;
    m_contextMenuRig->exec(ui->tableWownerod->viewport()->mapToGlobal(pos));
}

void XMRigWidget::wownerodLinkClicked() {
    QModelIndex index = ui->tableRig->currentIndex();
    auto download_link = m_urlsRig.at(index.row());
    Utils::externalLinkWarning(this, download_link);
}

void XMRigWidget::rigLinkClicked() {
    QModelIndex index = ui->tableRig->currentIndex();
    auto download_link = m_urlsRig.at(index.row());
    Utils::externalLinkWarning(this, download_link);
}

QStandardItemModel *XMRigWidget::model() {
    return m_modelRig;
}

XMRigWidget::~XMRigWidget() {
    delete ui;
}
