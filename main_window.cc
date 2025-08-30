#include <QApplication>
#include <QSplitter>
#include <QTabWidget>
#include <QStatusBar>
#include <QPushButton>
#include <QList>
#include <QInputDialog>
#include <QOverload>
#include <QtLogging>
#include <QDebug>

#include <nlohmann/json.hpp>

#include "main_window.hh"
#include "my_decompose.hh"
#include "asset_bag.hh"

using namespace Qt::Literals;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      network_thread_(),
      manager_(),
      splitter_(new QSplitter(Qt::Horizontal)),
      my_decompose_(new MyDecompose),
      tab_widget_(new QTabWidget),
      set_cookie_button_(new QPushButton(u"设置 Cookie"_s))
{
    setWindowTitle(u"我的小卡片 v%1 (Commit: %2, Date: %3)"_s.arg(qApp->applicationVersion())
                           .arg(GIT_COMMIT_HASH)
                           .arg(COMPILE_TIME));

    tab_widget_->setTabsClosable(true);
    connect(tab_widget_, &QTabWidget::tabCloseRequested, tab_widget_, &QTabWidget::removeTab);
    splitter_->addWidget(my_decompose_);
    splitter_->addWidget(tab_widget_);
    setCentralWidget(splitter_);

    connect(set_cookie_button_, &QPushButton::clicked, this, &MainWindow::onSetCookieButtonClicked);
    connect(my_decompose_, &MyDecompose::refreshRequested, this, [this]() {
        my_decompose_->clearMyDecomposeData();
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 1);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 2);
    });
    connect(my_decompose_, &MyDecompose::detailRequested, &manager_,
            qOverload<int, const QString &>(&BilibiliRequestManager::getAssetBag));
    connect(&manager_, &BilibiliRequestManager::myDecomposeDataReceived, this,
            &MainWindow::onMyDecomposeDataReceived);
    connect(&manager_, &BilibiliRequestManager::assetBagDataReceived, this,
            &MainWindow::onAssetBagDataReceived);

    {
        QStatusBar *status_bar = statusBar();
        status_bar->addPermanentWidget(set_cookie_button_);
    }

    manager_.moveToThread(&network_thread_);
    network_thread_.start();
}

MainWindow::~MainWindow()
{
    network_thread_.quit();
    network_thread_.wait();
}

void MainWindow::onSetCookieButtonClicked()
{
    bool ok;
    const QString cookie = QInputDialog::getText(this, u"输入 Cookie"_s, "Cookie"_L1,
                                                 QLineEdit::Normal, QString(), &ok);

    if (ok && !cookie.isEmpty()) {
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::setCookie, cookie);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 1);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 2);
    }
}

void MainWindow::onMyDecomposeDataReceived(int scene, const QByteArray &json)
{
    const nlohmann::json j = nlohmann::json::parse(json.toStdString(), nullptr, false);
    if (j.is_discarded()) {
        statusBar()->showMessage(u"json 非法"_s, 3000);
        return;
    }

    const int code = j.at("code");
    const std::string message = j.at("message");
    if (code != 0) {
        statusBar()->showMessage(u"code: %1, message: %2"_s.arg(code).arg(message), 3000);
        return;
    }

    auto &&data = j.at("data");
    auto &&list = data.at("list");
    if (list.is_null()) {
        qInfo() << "You don't have any card";
        statusBar()->showMessage(u"你没有任何卡"_s, 3000);
        return;
    }

    QList<MyDecomposeData> decompose_list;
    decompose_list.reserve(std::size(list)); // NOLINT(cppcoreguidelines-narrowing-conversions)
    for (auto &&a : list) {
        MyDecomposeData decompose_data = {
            QString::fromStdString(a.at("act_name").get<std::string>()),
            a.at("act_id").get<int>(),
            a.at("card_num").get<int>(),
        };
        decompose_list.emplace_back(std::move(decompose_data));
    }
    my_decompose_->setMyDecomposeData(scene, decompose_list);
}

void MainWindow::onAssetBagDataReceived(int act_id, const QString &act_name, int lottery_id,
                                        int ruid, const QByteArray &json)
{
    bool ok;
    const AssetBagData d = AssetBagData::fromJson(json, &ok);
    if (!ok) {
        statusBar()->showMessage(u"json 非法"_s, 3000);
        return;
    }

    AssetBag *asset_bag = new AssetBag;
    asset_bag->setInfo(act_id, act_name);
    asset_bag->setAssetBagData(d);
    tab_widget_->addTab(asset_bag, act_name);
    tab_widget_->setCurrentWidget(asset_bag);

    Q_UNUSED(act_id);
    Q_UNUSED(act_name);
    Q_UNUSED(lottery_id);
    Q_UNUSED(ruid);
    Q_UNUSED(json);
}