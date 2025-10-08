#include <QApplication>
#include <QScreen>
#include <QSplitter>
#include <QTabWidget>
#include <QStatusBar>
#include <QPushButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QList>
#include <QInputDialog>
#include <QOverload>
#include <QtLogging>
#include <QDebug>

#include "main_window.hh"
#include "my_favorite.hh"
#include "my_decompose.hh"
#include "asset_bag.hh"

using namespace Qt::Literals;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      settings_(u"conf.ini"_s, QSettings::Format::IniFormat),
      network_thread_(),
      manager_(),
      worker_(),
      splitter_(new QSplitter(Qt::Horizontal)),
      my_collection_(new QTabWidget),
      my_favorite_(new MyFavorite),
      my_decompose_(new MyDecompose),
      tab_widget_(new QTabWidget),
      set_cookie_button_(new QPushButton(u"设置 Cookie"_s)),
      save_cookie_check_box_(new QCheckBox(u"将 Cookie 存储在本地"_s))
{
    setWindowTitle(u"我的小卡片 v%1 (Commit: %2, Date: %3)"_s.arg(qApp->applicationVersion())
                           .arg(GIT_COMMIT_HASH)
                           .arg(COMPILE_TIME));

    tab_widget_->setTabsClosable(true);
    connect(tab_widget_, &QTabWidget::tabCloseRequested, this, [this](int index) {
        auto asset_bag = qobject_cast<AssetBag *>(tab_widget_->widget(index));
        Q_ASSERT(asset_bag != nullptr);
        if (map_.remove(ActIdAndLotteryId(asset_bag->actId(), asset_bag->lotteryId())) != 1) {
            Q_UNREACHABLE();
        }
        tab_widget_->removeTab(index);
    });

    my_collection_->addTab(my_favorite_, u"喜欢"_s);
    my_collection_->addTab(my_decompose_, u"拥有"_s);
    splitter_->addWidget(my_collection_);
    splitter_->addWidget(tab_widget_);
    setCentralWidget(splitter_);

    connect(set_cookie_button_, &QPushButton::clicked, this, &MainWindow::onSetCookieButtonClicked);
    connect(my_decompose_, &MyDecompose::refreshRequested, this, [this]() {
        my_decompose_->clearMyDecomposeData();
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 1);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 2);
    });
    connect(my_decompose_, &MyDecompose::exportRequested, this, &MainWindow::exportToCsvFile);
    connect(my_decompose_, &MyDecompose::detailRequested, &manager_,
            qOverload<int, const QString &>(&BilibiliRequestManager::getAssetBag));
    connect(&manager_, &BilibiliRequestManager::myDecomposeDataReceived, this,
            &MainWindow::onMyDecomposeDataReceived);
    connect(&manager_, &BilibiliRequestManager::assetBagDataReceived, this,
            &MainWindow::onAssetBagDataReceived);

    {
        QStatusBar *status_bar = statusBar();
        status_bar->addPermanentWidget(set_cookie_button_);
        status_bar->addPermanentWidget(save_cookie_check_box_);
    }

    manager_.moveToThread(&network_thread_);

    connect(&worker_, &CollectionExportWorker::progressChanged, this,
            [this](int current, int total) {
                statusBar()->showMessage(u"导出中... %1/%2"_s.arg(current).arg(total));
            });
    connect(&worker_, &CollectionExportWorker::finished, my_decompose_,
            &MyDecompose::enableExportButton);
    connect(&worker_, &CollectionExportWorker::finished, this,
            [this]() { statusBar()->showMessage(u"导出完成"_s, 3000); });
    worker_.moveToThread(&network_thread_);
    network_thread_.start();

    loadSettings();
}

MainWindow::~MainWindow()
{
    network_thread_.quit();
    network_thread_.wait();
}

void MainWindow::loadSettings()
{
    settings_.beginGroup("WindowSize");
    const int w = settings_.value("width", 600).toInt();
    const int h = settings_.value("height", 300).toInt();
    resize(w, h);
    settings_.endGroup();

    settings_.beginGroup("WindowPosition");
    if (QScreen *screen = qApp->primaryScreen()) {
        const int ax = settings_.value("x", (screen->size().width() - w) / 2).toInt();
        const int ay = settings_.value("y", (screen->size().height() - h) / 2).toInt();
        move(ax, ay);
    }
    settings_.endGroup();

    settings_.beginGroup("Splitter");
    if (settings_.contains("splitter_size")) {
        splitter_->restoreState(settings_.value("splitter_size").toByteArray());
    }
    settings_.endGroup();

    settings_.beginGroup("Network");
    save_cookie_check_box_->setChecked(settings_.value("save_cookie", false).toBool());
    if (settings_.contains("cookie")) {
        const QString cookie = settings_.value("cookie").toString();
        if (!cookie.isEmpty()) {
            QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::setCookie, cookie);
            QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 1);
            QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 2);
        }
    }
    settings_.endGroup();
}

void MainWindow::saveSettings()
{
    settings_.beginGroup("WindowSize");
    settings_.setValue("width", width());
    settings_.setValue("height", height());
    settings_.endGroup();

    settings_.beginGroup("WindowPosition");
    settings_.setValue("x", x());
    settings_.setValue("y", y());
    settings_.endGroup();

    settings_.beginGroup("Splitter");
    settings_.setValue("splitter_size", splitter_->saveState());
    settings_.endGroup();

    settings_.beginGroup("Network");
    settings_.setValue("save_cookie", save_cookie_check_box_->isChecked());
    if (save_cookie_check_box_->isChecked()) {
        QString cookie;
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::cookie,
                                  Qt::BlockingQueuedConnection, qReturnArg(cookie));
        settings_.setValue("cookie", cookie);
    } else {
        if (settings_.contains("cookie")) {
            settings_.remove("cookie");
        }
    }
    settings_.endGroup();
}

void MainWindow::exportToCsvFile()
{
    const QString file_name = QFileDialog::getSaveFileName(this, u"导出到 CSV 文件"_s,
                                                           qApp->applicationDirPath() % "/a.csv",
                                                           u"CSV 文件 (*.csv)"_s);
    if (file_name.isEmpty()) {
        statusBar()->showMessage(u"取消"_s, 3000);
        return;
    }

    my_decompose_->disableExportButton();
    QString cookie;
    QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::cookie,
                              Qt::BlockingQueuedConnection, qReturnArg(cookie));
    QMetaObject::invokeMethod(&worker_, &CollectionExportWorker::exportToCsvFile, file_name,
                              cookie);
}

void MainWindow::onSetCookieButtonClicked()
{
    bool ok;
    const QString cookie = QInputDialog::getText(this, u"输入 Cookie"_s, u"Cookie"_s,
                                                 QLineEdit::Normal, QString(), &ok);

    if (ok && !cookie.isEmpty()) {
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::setCookie, cookie);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 1);
        QMetaObject::invokeMethod(&manager_, &BilibiliRequestManager::getMyDecompose, 2);
    }
}

void MainWindow::onMyDecomposeDataReceived(int scene, const QByteArray &json)
{
    bool ok;
    const MyDecomposeData d = MyDecomposeData::fromJson(json, &ok);
    if (!ok) {
        statusBar()->showMessage(u"json 非法"_s, 3000);
        return;
    }

    my_decompose_->setMyDecomposeData(scene, d);
}

void MainWindow::onAssetBagDataReceived(int act_id, const QString &act_name,
                                        [[maybe_unused]] int lottery_id, [[maybe_unused]] int ruid,
                                        const QByteArray &json)
{
    bool ok;
    const AssetBagData d = AssetBagData::fromJson(json, &ok);
    if (!ok) {
        statusBar()->showMessage(u"json 非法"_s, 3000);
        return;
    }

    auto iter = map_.constFind(ActIdAndLotteryId(act_id, lottery_id));
    if (iter != map_.constEnd()) {
        AssetBag *asset_bag = iter.value();
        asset_bag->clearAssetBagData();
        asset_bag->setAssetBagData(d);
        tab_widget_->setCurrentWidget(asset_bag);
    } else {
        auto asset_bag = new AssetBag;
        map_.insert(ActIdAndLotteryId(act_id, lottery_id), asset_bag);
        asset_bag->setInfo(act_id, act_name);
        connect(asset_bag, &AssetBag::refreshRequested, &manager_,
                qOverload<int, const QString &, int>(&BilibiliRequestManager::getAssetBag));
        asset_bag->setAssetBagData(d);
        tab_widget_->addTab(asset_bag, act_name);
        tab_widget_->setCurrentWidget(asset_bag);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMetaObject::invokeMethod(&worker_, &CollectionExportWorker::stopAction,
                              Qt::BlockingQueuedConnection);
    QMainWindow::closeEvent(event);
}