#include <QFile>
#include <QTextStream>
#include <QtLogging>
#include <QDebug>
#include <QTimerEvent>

#include <chrono>

#include "collection_export_worker.hh"
#include "bilibili_request_manager.hh"
#include "my_decompose.hh"
#include "asset_bag.hh"

using namespace Qt::Literals;

CollectionExportWorker::CollectionExportWorker(QObject *parent)
    : QObject(parent),
      manager_(new BilibiliRequestManager(this)),
      file_(new QFile(this)),
      timer_id_(Qt::TimerId::Invalid),
      current_(),
      total_()
{
    connect(manager_, &BilibiliRequestManager::myDecomposeDataReceived, this,
            &CollectionExportWorker::onMyDecomposeDataReceived);
    connect(manager_, &BilibiliRequestManager::assetBagDataReceived, this,
            &CollectionExportWorker::onAssetBagDataReceived);
}

void CollectionExportWorker::exportToCsvFile(const QString &file_name, const QString &cookie)
{
    file_->setFileName(file_name);
    if (!file_->open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file:" << file_name;
        return;
    }

    QTextStream out(file_);
    out << QStringList{ u"收藏集名"_s, u"卡名"_s, u"稀有度"_s, u"编号"_s, u"是否限量"_s, }.join(',') << '\n';

    manager_->setCookie(cookie);
    manager_->getMyDecompose(1);
}

void CollectionExportWorker::onMyDecomposeDataReceived([[maybe_unused]] int scene,
                                                       const QByteArray &json)
{
    bool ok;
    const MyDecomposeData d = MyDecomposeData::fromJson(json, &ok);

    if (!ok) {
        emit finished();
        return;
    }

    if (!d.list.has_value()) {
        emit finished();
        return;
    }

    my_decompose_data_.reset(new MyDecomposeData(d));
    timer_id_ = static_cast<Qt::TimerId>(startTimer(std::chrono::milliseconds(300)));
    if (timer_id_ == Qt::TimerId::Invalid) {
        qWarning() << "Failed to start timer";
        emit finished();
        return;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions)
    total_ = my_decompose_data_->list->size();
}

void CollectionExportWorker::onAssetBagDataReceived([[maybe_unused]] int act_id,
                                                    const QString &act_name,
                                                    [[maybe_unused]] int lottery_id,
                                                    [[maybe_unused]] int ruid,
                                                    const QByteArray &json)
{
    bool ok;
    const AssetBagData d = AssetBagData::fromJson(json, &ok);

    if (!ok) {
        emit finished();
        return;
    }

    QTextStream out(file_);

    if (d.item_list.has_value()) {
        for (auto &&item : d.item_list.value()) {
            if (!item.card_item.has_value()) {
                continue;
            }

            if (item.card_item->card_id_list.has_value()) {
                for (auto &&card : item.card_item->card_id_list.value()) {
                    QStringList string_list;
                    string_list << act_name << item.card_item->card_name << item.scarcity()
                                << card.card_no
                                << (item.card_item->is_limited_card != 0 ? u"限量"_s : u"/"_s);
                    out << string_list.join(',') << '\n';
                }
            }
        }
    }

    if (d.collect_list.has_value()) {
        for (auto &&collect : d.collect_list.value()) {
            if (!collect.card_item.has_value() || !collect.card_item->card_type_info.has_value()) {
                continue;
            }

            if (collect.card_item->card_asset_info->card_item->card_id_list.has_value()) {
                for (auto &&card :
                     collect.card_item->card_asset_info->card_item->card_id_list.value()) {
                    QStringList string_list;
                    string_list << act_name << collect.card_item->card_type_info->name
                                << u"典藏卡"_s << card.card_no << u"/"_s;
                    out << string_list.join(',') << '\n';
                }
            }
        }
    }

    emit progressChanged(++current_, total_);
    if (current_ == total_) {
        emit finished();
    }
}

void CollectionExportWorker::timerEvent(QTimerEvent *event)
{
    if (timer_id_ == event->id()) {
        if (!my_decompose_data_->list->empty()) {
            manager_->getAssetBag(my_decompose_data_->list->back().act_id,
                                  my_decompose_data_->list->back().act_name);
            my_decompose_data_->list->pop_back();
        }
        if (my_decompose_data_->list->empty()) {
            killTimer(timer_id_);
        }
    }
    QObject::timerEvent(event);
}