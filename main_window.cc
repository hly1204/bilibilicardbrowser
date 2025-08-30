#include <QApplication>
#include <QSplitter>
#include <QStatusBar>
#include <QPushButton>
#include <QList>
#include <QInputDialog>
#include <QtLogging>
#include <QDebug>

#include <nlohmann/json.hpp>

#include "main_window.hh"
#include "my_decompose.hh"

using namespace Qt::Literals;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      network_thread_(),
      manager_(),
      splitter_(new QSplitter(Qt::Horizontal)),
      my_decompose_(new MyDecompose),
      set_cookie_button_(new QPushButton(u"设置 Cookie"_s))
{
    setWindowTitle(u"我的小卡片 v%1 (Commit: %2, Date: %3)"_s.arg(qApp->applicationVersion())
                           .arg(GIT_COMMIT_HASH)
                           .arg(COMPILE_TIME));

    splitter_->addWidget(my_decompose_);
    setCentralWidget(splitter_);

    connect(set_cookie_button_, &QPushButton::clicked, this, &MainWindow::onSetCookieButtonClicked);
    connect(my_decompose_, &MyDecompose::refreshRequested, this, [this]() {
        my_decompose_->clearMyDecomposeData();
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 1);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 2);
    });
    connect(&manager_, &BilibiliRequestManager::myDecomposeDataReceived, this,
            &MainWindow::onMyDecomposeDataReceived);

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

    const int code = j.at("code").get<int>();
    const std::string message = j.at("message").get<std::string>();
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
    decompose_list.reserve(std::size(list));
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