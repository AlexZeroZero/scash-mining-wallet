// Copyright (c) 2026 The Scash integrated wallet developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_MININGPAGE_H
#define BITCOIN_QT_MININGPAGE_H

#include <QWidget>

class PlatformStyle;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLineEdit;
class QNetworkAccessManager;
class QNetworkReply;
class QPlainTextEdit;
class QProcess;
class QPushButton;
class QSpinBox;
class QLabel;
class QTimer;
class WalletModel;

class MiningPage : public QWidget
{
    Q_OBJECT

public:
    explicit MiningPage(const PlatformStyle* platform_style, QWidget* parent = nullptr);
    ~MiningPage();
    void setModel(WalletModel* model);

private Q_SLOTS:
    void poolChanged(int index);
    void createReceivingAddress();
    void toggleMining();
    void readMinerOutput();
    void minerFinished(int exit_code);
    void openPoolDashboard();
    void openBlockExplorer();
    void toggleLanguage();
    void refreshPrice();
    void priceReplyFinished(QNetworkReply* reply);

private:
    QString minerPath() const;
    void appendLog(const QString& text);
    void setRunning(bool running);
    void retranslatePage();
    void stopMinerTree();

    bool m_chinese{false};
    bool m_stopping{false};
    WalletModel* m_model{nullptr};
    QProcess* m_miner{nullptr};
    QComboBox* m_pool{nullptr};
    QLineEdit* m_pool_endpoint{nullptr};
    QLineEdit* m_address{nullptr};
    QLineEdit* m_worker{nullptr};
    QLineEdit* m_dashboard_template{nullptr};
    QSpinBox* m_threads{nullptr};
    QLabel* m_hashrate{nullptr};
    QLabel* m_price{nullptr};
    QLabel* m_status{nullptr};
    QPlainTextEdit* m_output{nullptr};
    QPushButton* m_start{nullptr};
    QPushButton* m_language{nullptr};
    QPushButton* m_new_address{nullptr};
    QPushButton* m_dashboard{nullptr};
    QPushButton* m_explorer{nullptr};
    QLabel* m_title{nullptr};
    QLabel* m_description{nullptr};
    QLabel* m_pool_label{nullptr};
    QLabel* m_endpoint_label{nullptr};
    QLabel* m_address_label{nullptr};
    QLabel* m_worker_label{nullptr};
    QLabel* m_threads_label{nullptr};
    QLabel* m_dashboard_label{nullptr};
    QGroupBox* m_settings_group{nullptr};
    QGroupBox* m_status_group{nullptr};
    QNetworkAccessManager* m_price_manager{nullptr};
    QTimer* m_price_timer{nullptr};
};

#endif // BITCOIN_QT_MININGPAGE_H
