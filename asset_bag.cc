#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QResizeEvent>
#include <QtLogging>
#include <QDebug>

#include <nlohmann/json.hpp>

#include "asset_bag.hh"

using namespace Qt::Literals;

template <typename Tp>
void from_json(const nlohmann::json &j, QList<Tp> &list)
{
    list.clear();
    if (!j.is_array()) {
        return;
    }

    list.reserve(std::size(j));
    for (auto &&item : j) {
        auto &&last_item = list.emplace_back();
        item.get_to(last_item);
    }
}

void from_json(const nlohmann::json &j, QString &s)
{
    s = QString::fromStdString(j.get<std::string>());
}
void from_json(const nlohmann::json &j, QUrl &u)
{
    u.setUrl(j.get<QString>());
}
void from_json(const nlohmann::json &j, QDateTime &d)
{
    d = QDateTime::fromSecsSinceEpoch(j.get<qint64>());
}

void from_json(const nlohmann::json &j, AssetBagData::CardIdListItem &item)
{
    j.at("card_id").get_to(item.card_id);
    j.at("card_no").get_to(item.card_no);
    j.at("status").get_to(item.status);
    j.at("card_right").at("is_transfer").get_to(item.card_right.is_transfer);
}

void from_json(const nlohmann::json &j, AssetBagData::ListItem::CardItem &item)
{
    j.at("card_type_id").get_to(item.card_type_id);
    j.at("card_name").get_to(item.card_name);
    j.at("card_img").get_to(item.card_img);
    j.at("card_type").get_to(item.card_type);
    item.card_id_list.reset();
    auto &&card_id_list = j.at("card_id_list");
    if (card_id_list.is_array()) {
        card_id_list.get_to(item.card_id_list.emplace());
    }
    j.at("total_cnt").get_to(item.total_cnt);
    j.at("total_cnt_show").get_to(item.total_cnt_show);
    j.at("holding_rate").get_to(item.holding_rate);
    j.at("card_scarcity").get_to(item.card_scarcity);
    j.at("is_limited_card").get_to(item.is_limited_card);
}

void from_json(const nlohmann::json &j, AssetBagData::ListItem &item)
{
    j.at("item_type").get_to(item.item_type);
    j.at("item_scarcity").get_to(item.item_scarcity);
    item.card_item.reset();
    auto &&card_item = j.at("card_item");
    if (!card_item.is_null()) {
        card_item.get_to(item.card_item.emplace());
    }
}

void from_json(const nlohmann::json &j, AssetBagData::CollectListItem::CardItem::CardTypeInfo &info)
{
    j.at("id").get_to(info.id);
    j.at("name").get_to(info.name);
    j.at("overview_image").get_to(info.overview_image);
    j.at("scarcity").get_to(info.scarcity);
}

void from_json(const nlohmann::json &j, AssetBagData::CollectListItem::CardItem &item)
{
    auto &&card_type_info = j.at("card_type_info");
    if (!card_type_info.is_null()) {
        card_type_info.get_to(item.card_type_info.emplace());
    }
    auto &&card_asset_info = j.at("card_asset_info");
    if (!card_asset_info.is_null()) {
        card_asset_info.get_to(item.card_asset_info.emplace());
    }
}

void from_json(const nlohmann::json &j, AssetBagData::CollectListItem &item)
{
    j.at("collect_id").get_to(item.collect_id);
    j.at("start_time").get_to(item.start_time);
    j.at("end_time").get_to(item.end_time);
    j.at("redeem_text").get_to(item.redeem_text);
    j.at("redeem_item_type").get_to(item.redeem_item_type);
    j.at("redeem_item_id").get_to(item.redeem_item_id);
    j.at("redeem_item_name").get_to(item.redeem_item_name);
    j.at("redeem_item_image").get_to(item.redeem_item_image);
    j.at("owned_item_amount").get_to(item.owned_item_amount);
    j.at("require_item_amount").get_to(item.require_item_amount);
    j.at("has_redeemed_cnt").get_to(item.has_redeemed_cnt);
    j.at("effective_forever").get_to(item.effective_forever);
    item.card_item.reset();
    auto &&card_item = j.at("card_item");
    if (!card_item.is_null()) {
        card_item.get_to(item.card_item.emplace());
    }
}

void from_json(const nlohmann::json &j, AssetBagData::LotterySimpleListItem &item)
{
    j.at("lottery_id").get_to(item.lottery_id);
    j.at("lottery_name").get_to(item.lottery_name);
}

void from_json(const nlohmann::json &j, AssetBagData &data)
{
    j.at("total_item_cnt").get_to(data.total_item_cnt);
    j.at("owned_item_cnt").get_to(data.owned_item_cnt);
    data.item_list.reset();
    auto &&item_list = j.at("item_list");
    if (item_list.is_array()) {
        item_list.get_to(data.item_list.emplace());
    }
    data.collect_list.reset();
    auto &&collect_list = j.at("collect_list");
    if (collect_list.is_array()) {
        collect_list.get_to(data.collect_list.emplace());
    }
    data.lottery_simple_list.reset();
    auto &&lottery_simple_list = j.at("lottery_simple_list");
    if (lottery_simple_list.is_array()) {
        lottery_simple_list.get_to(data.lottery_simple_list.emplace());
    }
}

AssetBagData AssetBagData::fromJson(const QByteArray &json, bool *ok)
{
    if (ok) {
        *ok = false;
    }

    AssetBagData d;

    do {
        const nlohmann::json j = nlohmann::json::parse(json.toStdString(), nullptr, false);
        if (j.is_discarded()) {
            break;
        }

        const int code = j.at("code").get<int>();
        const std::string message = j.at("message").get<std::string>();
        if (code != 0) {
            qWarning() << "code:" << code << "message:" << message;
            break;
        }

        auto &&data = j.at("data");
        if (!data.is_object()) {
            break;
        }
        data.get_to(d);

        if (ok) {
            *ok = true;
        }
    } while (false);

    return d;
}

AssetBag::AssetBag(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      act_id_(),
      lottery_id_(),
      link_label_(new QLabel(u"未知收藏集"_s, this)),
      item_cnt_label_(new QLabel(u"拥有/总计: %1/%2"_s.arg(0).arg(1), this)),
      tree_widget_(new QTreeWidget(this)),
      refresh_button_(new QPushButton(u"刷新"_s, this))
{
    link_label_->adjustSize();
    item_cnt_label_->adjustSize();
    item_cnt_label_->move(link_label_->geometry().topRight() + QPoint(2, 0));
    refresh_button_->adjustSize();
    link_label_->setTextInteractionFlags(Qt::TextBrowserInteraction);
    link_label_->setOpenExternalLinks(true);
    tree_widget_->move(link_label_->geometry().bottomLeft() + QPoint(0, 1));

    connect(refresh_button_, &QPushButton::clicked, this, [this]() {
        if (act_id_ == 0) {
            return;
        }
        emit refreshRequested(act_id_, lottery_id_, act_name_, lottery_name_);
    });

    tree_widget_->setColumnCount(6);
    tree_widget_->setHeaderLabels(QStringList{
            u"稀有度"_s,
            u"名称"_s,
            u"编号/总数"_s,
            u"持有率"_s,
            u"限量卡"_s,
            u"其他状态"_s,
    });
}

void AssetBag::setInfo(int act_id, int lottery_id, const QString &act_name,
                       const QString &lottery_name)
{
    act_id_ = act_id;
    lottery_id_ = lottery_id;
    act_name_ = act_name;
    lottery_name_ = lottery_name;

    // 若 lottery_id = 0 则 lottery_name = "全部奖池"
    link_label_->setText(
            u"<a href=\"https://www.bilibili.com/h5/mall/digital-card/home?-Abrowser=live&act_id=%1&hybrid_set_header=2&lottery_id=%2\">%3</a>"_s
                    .arg(act_id)
                    .arg(lottery_id)
                    .arg(act_name % " - " % lottery_name));
    link_label_->adjustSize();
    item_cnt_label_->move(link_label_->geometry().topRight() + QPoint(2, 0));
}

void AssetBag::setInfo(int act_id, const QString &act_name)
{
    setInfo(act_id, 0, act_name, u"全部奖池"_s);
}

void AssetBag::clearAssetBagData() // NOLINT(readability-convert-member-functions-to-static)
{
    Q_UNIMPLEMENTED();
}

void AssetBag::setAssetBagData(const AssetBagData &data)
{
    item_cnt_label_->setText(
            u"拥有/总计: %1/%2"_s.arg(data.owned_item_cnt).arg(data.total_item_cnt));
    item_cnt_label_->adjustSize();

    // 可以抽到的卡片
    if (data.item_list.has_value()) {
        for (auto &&item : *data.item_list) {
            if (!item.card_item.has_value()) {
                continue;
            }
            QTreeWidgetItem *top_item = new QTreeWidgetItem;
            tree_widget_->addTopLevelItem(top_item);
            tree_widget_->setItemWidget(top_item, 0, new QLabel(item.scarcity()));
            tree_widget_->setItemWidget(top_item, 1, new QLabel(item.card_item->card_name));
            tree_widget_->setItemWidget(top_item, 2,
                                        new QLabel(QString::number(item.card_item->total_cnt)));
            tree_widget_->setItemWidget(
                    top_item, 3,
                    new QLabel(QString::number(item.card_item->holding_rate / 100.0, 'g', 2)
                               % '%'));
            tree_widget_->setItemWidget(
                    top_item, 4,
                    new QLabel(item.card_item->is_limited_card != 0 ? u"限量"_s : u"无限制"_s));

            if (item.card_item->card_id_list.has_value()) {
                for (auto &&card : *item.card_item->card_id_list) {
                    QTreeWidgetItem *sub_item = new QTreeWidgetItem(top_item);
                    tree_widget_->setItemWidget(sub_item, 1, new QLabel(item.card_item->card_name));
                    tree_widget_->setItemWidget(sub_item, 2, new QLabel(card.card_no));
                    tree_widget_->setItemWidget(
                            sub_item, 5,
                            new QLabel(card.card_right.is_transfer != 0 ? u"转赠中"_s : u""_s));
                }
                top_item->setExpanded(true);
            }
        }
    }

    // 典藏卡
    if (data.collect_list.has_value()) {
        for (auto &&collect : *data.collect_list) {
            if (!collect.card_item.has_value() || !collect.card_item->card_type_info.has_value()) {
                continue;
            }
            QTreeWidgetItem *top_item = new QTreeWidgetItem;
            tree_widget_->addTopLevelItem(top_item);
            tree_widget_->setItemWidget(top_item, 0, new QLabel(u"典藏卡"_s));
            tree_widget_->setItemWidget(top_item, 1,
                                        new QLabel(collect.card_item->card_type_info->name));
            tree_widget_->setItemWidget(
                    top_item, 2,
                    new QLabel(QString::number(
                            collect.card_item->card_asset_info->card_item->total_cnt)));
            tree_widget_->setItemWidget(top_item, 3, new QLabel(u"/"_s));
            tree_widget_->setItemWidget(top_item, 4, new QLabel(u"/"_s));

            if (collect.card_item->card_asset_info->card_item->card_id_list.has_value()) {
                for (auto &&card : *collect.card_item->card_asset_info->card_item->card_id_list) {
                    QTreeWidgetItem *sub_item = new QTreeWidgetItem(top_item);
                    tree_widget_->setItemWidget(
                            sub_item, 1, new QLabel(collect.card_item->card_type_info->name));
                    tree_widget_->setItemWidget(sub_item, 2, new QLabel(card.card_no));
                    tree_widget_->setItemWidget(
                            sub_item, 5,
                            new QLabel(card.card_right.is_transfer != 0 ? u"转赠中"_s : u""_s));
                }
                top_item->setExpanded(true);
            }
        }
    }

    tree_widget_->resizeColumnToContents(1);
    tree_widget_->resizeColumnToContents(2);
}

void AssetBag::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const QSize size = event->size();
    tree_widget_->resize(size.width(),
                         size.height() - 2 - link_label_->height() - 2 - refresh_button_->height());
    refresh_button_->move(tree_widget_->geometry().bottomLeft() + QPoint(0, 1));
}