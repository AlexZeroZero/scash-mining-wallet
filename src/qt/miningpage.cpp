// Copyright (c) 2026 The Scash integrated wallet developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/miningpage.h>

#include <qt/addresstablemodel.h>
#include <qt/platformstyle.h>
#include <qt/walletmodel.h>

#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>

MiningPage::MiningPage(const PlatformStyle* platform_style, QWidget* parent)
    : QWidget(parent), m_miner(new QProcess(this))
{
    auto* main_layout = new QVBoxLayout(this);
    auto* title_row = new QHBoxLayout();
    m_title = new QLabel(this);
    QFont title_font = m_title->font();
    title_font.setPointSize(title_font.pointSize() + 4);
    title_font.setBold(true);
    m_title->setFont(title_font);
    m_language = new QPushButton(this);
    m_language->setFixedWidth(110);
    title_row->addWidget(m_title);
    title_row->addStretch();
    title_row->addWidget(m_language);
    main_layout->addLayout(title_row);
    m_description = new QLabel(this);
    main_layout->addWidget(m_description);

    m_settings_group = new QGroupBox(this);
    auto* form = new QFormLayout(m_settings_group);
    m_pool = new QComboBox(m_settings_group);
    const auto add_pool = [this](const QString& name, const QString& endpoint, const QString& dashboard) {
        m_pool->addItem(name, endpoint);
        m_pool->setItemData(m_pool->count() - 1, dashboard, Qt::UserRole + 1);
    };
    add_pool(QStringLiteral("RPlant"), QStringLiteral("stratum+tcp://eu.rplant.xyz:7019"),
             QStringLiteral("https://pool.rplant.xyz/#satoshicash/miner/{address}"));
    add_pool(QStringLiteral("SCASH.pro (PPLNS)"), QStringLiteral("stratum+tcp://pool.scash.pro:8888"),
             QStringLiteral("https://scash.pro/"));
    add_pool(QStringLiteral("Cédric Crispin"), QStringLiteral("stratum+tcp://satoshicash.cedric-crispin.com:4474"),
             QStringLiteral("https://satoshicash.cedric-crispin.com/"));
    add_pool(QStringLiteral("scash.work (SOLO)"), QString(), QStringLiteral("https://scash.work/"));
    add_pool(QStringLiteral("RBMPool"), QString(), QStringLiteral("https://rbmpool.com/"));
    add_pool(QStringLiteral("ICMiners"), QString(), QStringLiteral("https://icminers.com/pool/scash-shared"));
    add_pool(QStringLiteral("MagicalGirlsUnite"), QString(), QStringLiteral("https://magicalgirlsunite.moe/"));
    add_pool(QStringLiteral("TazMining (SOLO)"), QString(), QStringLiteral("https://tazmining.ch/"));
    add_pool(tr("Custom pool"), QString(), QString());
    m_pool_endpoint = new QLineEdit(m_settings_group);
    m_pool_endpoint->setPlaceholderText(QStringLiteral("stratum+tcp://host:port"));
    m_address = new QLineEdit(m_settings_group);
    m_address->setPlaceholderText(tr("SCASH receiving address"));
    auto* address_row = new QWidget(m_settings_group);
    auto* address_layout = new QHBoxLayout(address_row);
    address_layout->setContentsMargins(0, 0, 0, 0);
    address_layout->addWidget(m_address);
    m_new_address = new QPushButton(address_row);
    address_layout->addWidget(m_new_address);
    m_worker = new QLineEdit(QStringLiteral("scash-wallet"), m_settings_group);
    m_threads = new QSpinBox(m_settings_group);
    m_threads->setRange(1, qMax(1, QThread::idealThreadCount()));
    m_threads->setValue(qMax(1, QThread::idealThreadCount() / 2));
    m_threads->setSuffix(tr(" threads"));
    m_dashboard_template = new QLineEdit(m_settings_group);
    m_dashboard_template->setPlaceholderText(tr("Pool dashboard URL; use {address} as placeholder"));
    m_pool_label = new QLabel(m_settings_group);
    m_endpoint_label = new QLabel(m_settings_group);
    m_address_label = new QLabel(m_settings_group);
    m_worker_label = new QLabel(m_settings_group);
    m_threads_label = new QLabel(m_settings_group);
    m_dashboard_label = new QLabel(m_settings_group);
    form->addRow(m_pool_label, m_pool);
    form->addRow(m_endpoint_label, m_pool_endpoint);
    form->addRow(m_address_label, address_row);
    form->addRow(m_worker_label, m_worker);
    form->addRow(m_threads_label, m_threads);
    form->addRow(m_dashboard_label, m_dashboard_template);
    main_layout->addWidget(m_settings_group);

    m_status_group = new QGroupBox(this);
    auto* status_layout = new QVBoxLayout(m_status_group);
    auto* summary = new QHBoxLayout();
    m_status = new QLabel(m_status_group);
    m_hashrate = new QLabel(m_status_group);
    m_hashrate->setStyleSheet(QStringLiteral("QLabel { color: #9C27D7; font-weight: 600; }"));
    summary->addWidget(m_status);
    summary->addStretch();
    summary->addWidget(m_hashrate);
    status_layout->addLayout(summary);
    m_output = new QPlainTextEdit(m_status_group);
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(1000);
    m_output->setMinimumHeight(160);
    status_layout->addWidget(m_output);
    main_layout->addWidget(m_status_group, 1);

    auto* buttons = new QHBoxLayout();
    m_start = new QPushButton(this);
    if (platform_style->getImagesOnButtons()) m_start->setIcon(platform_style->SingleColorIcon(QStringLiteral(":/icons/network")));
    m_dashboard = new QPushButton(this);
    m_explorer = new QPushButton(this);
    m_price = new QLabel(this);
    m_price->setStyleSheet(QStringLiteral("QLabel { color: #9C27D7; font-weight: 600; }"));
    buttons->addWidget(m_start);
    buttons->addWidget(m_dashboard);
    buttons->addWidget(m_explorer);
    buttons->addWidget(m_price);
    buttons->addStretch();
    main_layout->addLayout(buttons);

    connect(m_pool, qOverload<int>(&QComboBox::currentIndexChanged), this, &MiningPage::poolChanged);
    connect(m_language, &QPushButton::clicked, this, &MiningPage::toggleLanguage);
    connect(m_new_address, &QPushButton::clicked, this, &MiningPage::createReceivingAddress);
    connect(m_start, &QPushButton::clicked, this, &MiningPage::toggleMining);
    connect(m_dashboard, &QPushButton::clicked, this, &MiningPage::openPoolDashboard);
    connect(m_explorer, &QPushButton::clicked, this, &MiningPage::openBlockExplorer);
    m_price_manager = new QNetworkAccessManager(this);
    m_price_timer = new QTimer(this);
    m_price_timer->setInterval(60000);
    connect(m_price_timer, &QTimer::timeout, this, &MiningPage::refreshPrice);
    connect(m_price_manager, &QNetworkAccessManager::finished, this, &MiningPage::priceReplyFinished);
    connect(m_miner, &QProcess::readyReadStandardOutput, this, &MiningPage::readMinerOutput);
    connect(m_miner, &QProcess::readyReadStandardError, this, &MiningPage::readMinerOutput);
    connect(m_miner, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int code, QProcess::ExitStatus) { minerFinished(code); });
    poolChanged(0);
    retranslatePage();
    m_price_timer->start();
    QTimer::singleShot(0, this, &MiningPage::refreshPrice);
}

MiningPage::~MiningPage()
{
    if (m_miner->state() != QProcess::NotRunning) {
        stopMinerTree();
        m_miner->waitForFinished(3000);
    }
}

void MiningPage::setModel(WalletModel* model)
{
    m_model = model;
}

void MiningPage::poolChanged(int index)
{
    const QString endpoint = m_pool->itemData(index, Qt::UserRole).toString();
    const QString dashboard = m_pool->itemData(index, Qt::UserRole + 1).toString();
    m_pool_endpoint->setReadOnly(!endpoint.isEmpty());
    m_pool_endpoint->setText(endpoint);
    m_dashboard_template->setText(dashboard);
}

void MiningPage::createReceivingAddress()
{
    if (!m_model) return;
    const QString address = m_model->getAddressTableModel()->addRow(
        AddressTableModel::Receive, tr("Mining"), QString(), m_model->wallet().getDefaultAddressType());
    if (address.isEmpty()) {
        QMessageBox::warning(this, tr("Mining"), tr("Could not create a receiving address. Unlock the wallet and try again."));
        return;
    }
    m_address->setText(address);
}

QString MiningPage::minerPath() const
{
#ifdef Q_OS_WIN
    const QString name = QStringLiteral("xmrigDaemon.exe");
#else
    const QString name = QStringLiteral("xmrigDaemon");
#endif
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("miner/") + name);
}

void MiningPage::toggleMining()
{
    if (m_miner->state() != QProcess::NotRunning) {
        if (m_stopping) return;
        m_stopping = true;
        appendLog(tr("Stopping miner..."));
        m_start->setEnabled(false);
        retranslatePage();
        stopMinerTree();
        QTimer::singleShot(2000, this, [this] {
            if (m_miner->state() != QProcess::NotRunning) {
                appendLog(m_chinese ? QStringLiteral("矿工未响应，正在强制停止...") : QStringLiteral("Miner did not respond; forcing shutdown..."));
                m_miner->kill();
            }
        });
        return;
    }
    if (m_pool_endpoint->text().trimmed().isEmpty() || m_address->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Mining"), tr("Select a pool and enter or create a payout address first."));
        return;
    }
    const QString executable = minerPath();
    if (!QFileInfo::exists(executable)) {
        QMessageBox::critical(this, tr("Miner not found"),
            tr("The bundled miner was not found at:\n%1\n\nReinstall the complete wallet package.").arg(QDir::toNativeSeparators(executable)));
        return;
    }

    QString user = m_address->text().trimmed();
    if (!m_worker->text().trimmed().isEmpty()) user += QStringLiteral(".") + m_worker->text().trimmed();
    const QStringList args{
        QStringLiteral("-a"), QStringLiteral("rx/scash"),
        QStringLiteral("-o"), m_pool_endpoint->text().trimmed(),
        QStringLiteral("-u"), user,
        QStringLiteral("-p"), QStringLiteral("x"),
        QStringLiteral("-t"), QString::number(m_threads->value()),
        QStringLiteral("--print-time=5"),
    };
    m_output->clear();
    appendLog(tr("Starting xmrigCC with %1 CPU thread(s)...").arg(m_threads->value()));
    m_miner->setProgram(executable);
    m_miner->setArguments(args);
    m_miner->setWorkingDirectory(QFileInfo(executable).absolutePath());
    m_miner->start();
    if (!m_miner->waitForStarted(3000)) {
        appendLog(tr("Failed to start: %1").arg(m_miner->errorString()));
        return;
    }
    setRunning(true);
}

void MiningPage::readMinerOutput()
{
    const QString text = QString::fromLocal8Bit(m_miner->readAllStandardOutput()) +
                         QString::fromLocal8Bit(m_miner->readAllStandardError());
    appendLog(text.trimmed());
    static const QRegularExpression rate_re(QStringLiteral("(\\d+(?:\\.\\d+)?)\\s*([kMGT]?)H/s"),
                                            QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator matches = rate_re.globalMatch(text);
    QRegularExpressionMatch last;
    while (matches.hasNext()) last = matches.next();
    if (last.hasMatch()) {
        const QString prefix = m_chinese ? QStringLiteral("算力：") : QStringLiteral("Hashrate: ");
        m_hashrate->setText(prefix + last.captured(1) + QStringLiteral(" ") + last.captured(2) + QStringLiteral("H/s"));
    }
}

void MiningPage::minerFinished(int exit_code)
{
    appendLog(tr("Miner exited with code %1.").arg(exit_code));
    m_stopping = false;
    m_start->setEnabled(true);
    setRunning(false);
}

void MiningPage::openPoolDashboard()
{
    if (m_address->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Mining"), tr("Enter or create a payout address first."));
        return;
    }
    QString url = m_dashboard_template->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, tr("Mining"), tr("Enter the selected pool's address dashboard URL first."));
        return;
    }
    url.replace(QStringLiteral("{address}"), QString::fromUtf8(QUrl::toPercentEncoding(m_address->text().trimmed())));
    QDesktopServices::openUrl(QUrl(url));
}

void MiningPage::openBlockExplorer()
{
    const QString address = m_address->text().trimmed();
    if (address.isEmpty()) {
        QMessageBox::warning(this,
            m_chinese ? QStringLiteral("区块浏览器") : QStringLiteral("Block explorer"),
            m_chinese ? QStringLiteral("请先输入或创建收益地址。") : QStringLiteral("Enter or create a payout address first."));
        return;
    }
    const QString url = QStringLiteral("https://explorer.scash.network/address/") +
                        QString::fromUtf8(QUrl::toPercentEncoding(address));
    QDesktopServices::openUrl(QUrl(url));
}

void MiningPage::refreshPrice()
{
    QNetworkRequest request{QUrl(QStringLiteral(
        "https://api.coingecko.com/api/v3/simple/price"
        "?ids=satoshi-cash-network&vs_currencies=usd"))};
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "Scash-Qt-Wallet/1.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_price_manager->get(request);
}

void MiningPage::priceReplyFinished(QNetworkReply* reply)
{
    const QByteArray payload = reply->readAll();
    const bool success = reply->error() == QNetworkReply::NoError;
    reply->deleteLater();
    if (!success) {
        if (m_price->property("hasPrice").toBool()) return;
        m_price->setText(m_chinese ? QStringLiteral("SCASH 价格：暂不可用")
                                   : QStringLiteral("SCASH price: unavailable"));
        return;
    }

    const QJsonObject coin = QJsonDocument::fromJson(payload).object()
                                 .value(QStringLiteral("satoshi-cash-network")).toObject();
    const double usd = coin.value(QStringLiteral("usd")).toDouble();
    if (usd <= 0.0) return;
    const int decimals = usd < 0.01 ? 6 : 4;
    m_price->setProperty("hasPrice", true);
    m_price->setProperty("usdPrice", usd);
    m_price->setText(QStringLiteral("SCASH: $%1 USD").arg(QLocale::c().toString(usd, 'f', decimals)));
}

void MiningPage::appendLog(const QString& text)
{
    if (!text.isEmpty()) m_output->appendPlainText(text);
}

void MiningPage::setRunning(bool running)
{
    m_pool->setEnabled(!running);
    m_pool_endpoint->setEnabled(!running);
    m_address->setEnabled(!running);
    m_worker->setEnabled(!running);
    m_threads->setEnabled(!running);
    retranslatePage();
}

void MiningPage::stopMinerTree()
{
#ifdef Q_OS_WIN
    const qint64 pid = m_miner->processId();
    if (pid > 0) {
        // QProcess::kill() only terminates the direct process on Windows.
        // xmrigCC can create worker children, so terminate the complete tree.
        QProcess::execute(QStringLiteral("taskkill.exe"),
                          {QStringLiteral("/PID"), QString::number(pid), QStringLiteral("/T"), QStringLiteral("/F")});
    }
#else
    m_miner->terminate();
#endif
}

void MiningPage::toggleLanguage()
{
    m_chinese = !m_chinese;
    retranslatePage();
}

void MiningPage::retranslatePage()
{
    const bool running = m_miner->state() != QProcess::NotRunning;
    m_title->setText(m_chinese ? QStringLiteral("CPU 挖矿") : QStringLiteral("CPU Mining"));
    m_description->setText(m_chinese
        ? QStringLiteral("使用钱包内置的xmrigCC挖掘SCASH。挖矿会占用较多CPU和电力、请将电脑设置为高性能模式、并开启大锁定内存页、这样可以使挖矿效率上升。")
        : QStringLiteral("Mine SCASH with the bundled xmrigCC miner. Mining uses significant CPU and power."));
    m_language->setText(m_chinese ? QStringLiteral("English") : QStringLiteral("中文"));
    m_settings_group->setTitle(m_chinese ? QStringLiteral("挖矿设置") : QStringLiteral("Mining settings"));
    m_status_group->setTitle(m_chinese ? QStringLiteral("实时挖矿数据") : QStringLiteral("Live mining data"));
    m_pool_label->setText(m_chinese ? QStringLiteral("矿池：") : QStringLiteral("Pool:"));
    m_endpoint_label->setText(m_chinese ? QStringLiteral("矿池地址：") : QStringLiteral("Pool endpoint:"));
    m_address_label->setText(m_chinese ? QStringLiteral("收益地址：") : QStringLiteral("Payout address:"));
    m_worker_label->setText(m_chinese ? QStringLiteral("矿工名称：") : QStringLiteral("Worker name:"));
    m_threads_label->setText(m_chinese ? QStringLiteral("CPU 核心数：") : QStringLiteral("CPU allocation:"));
    m_dashboard_label->setText(m_chinese ? QStringLiteral("地址查询页面：") : QStringLiteral("Address dashboard:"));
    m_new_address->setText(m_chinese ? QStringLiteral("新建钱包地址") : QStringLiteral("New wallet address"));
    m_dashboard->setText(m_chinese ? QStringLiteral("在矿池查询地址") : QStringLiteral("View address at pool"));
    m_explorer->setText(m_chinese ? QStringLiteral("区块浏览器") : QStringLiteral("Block explorer"));
    if (!m_price->property("hasPrice").toBool()) {
        m_price->setText(m_chinese ? QStringLiteral("SCASH 价格：获取中...")
                                   : QStringLiteral("SCASH price: loading..."));
    }
    m_status->setText(m_stopping
        ? (m_chinese ? QStringLiteral("正在停止...") : QStringLiteral("Stopping..."))
        : running ? (m_chinese ? QStringLiteral("正在挖矿") : QStringLiteral("Mining"))
        : (m_chinese ? QStringLiteral("已停止") : QStringLiteral("Stopped")));
    m_start->setText(m_stopping
        ? (m_chinese ? QStringLiteral("正在停止...") : QStringLiteral("Stopping..."))
        : running ? (m_chinese ? QStringLiteral("停止挖矿") : QStringLiteral("Stop mining"))
        : (m_chinese ? QStringLiteral("开始挖矿") : QStringLiteral("Start mining")));
    m_start->setEnabled(!m_stopping);
    if (!running) m_hashrate->setText(m_chinese ? QStringLiteral("算力：-- H/s") : QStringLiteral("Hashrate: -- H/s"));
    m_threads->setSuffix(m_chinese ? QStringLiteral(" 核") : QStringLiteral(" threads"));
}
