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

    QPixmap p(":assets/images/fire.png");
    ui->lbl_logo->setPixmap(p.scaled(268, 271, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_reward->hide();

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

    // defaults
    ui->btn_stop->setEnabled(false);
    ui->check_autoscroll->setChecked(true);
#ifdef Q_OS_WIN
    ui->label_path->setText("Path to wownerod.exe");
#endif
    ui->label_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_status->hide();

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

void XMRigWidget::onBlockReward() {
    QDateTime date = QDateTime::currentDateTime();
    QString formattedTime = date.toString("yyyy/MM/dd hh:mm");
    ui->lbl_reward->setText(QString("Congrats: new block found at %1").arg(formattedTime));
    ui->lbl_reward->show();
}

void XMRigWidget::onClearClicked() {
    ui->console->clear();
}

void XMRigWidget::onStartClicked() {
    auto binPath = config()->get(Config::wownerodPath).toString();
    if(!m_ctx->XMRig->start(binPath, m_threads)) return;

    ui->btn_start->setEnabled(false);
    ui->btn_stop->setEnabled(true);
    emit miningStarted();
}

void XMRigWidget::onStopClicked() {
    m_ctx->XMRig->stop();
}

void XMRigWidget::onProcessOutput(const QByteArray &data) {
    auto output = Utils::barrayToString(data);
    if(output.endsWith("\n"))
        output = output.trimmed();

    this->appendText(output);

    if(ui->check_autoscroll->isChecked())
        ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
}

void XMRigWidget::onStopped() {
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    ui->label_status->hide();
    emit miningEnded();
}

void XMRigWidget::onProcessError(const QString &msg) {
    this->appendText(msg);
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    ui->label_status->hide();
    emit miningEnded();
}

void XMRigWidget::onHashrate(const QString &hashrate) {
    ui->label_status->show();
    ui->label_status->setText(QString("Mining at %1").arg(hashrate));
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
