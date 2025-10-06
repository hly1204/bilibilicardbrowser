#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QResizeEvent>
#include <QtMinMax>
#include <QtLogging>
#include <QDebug>
#include <QtAssert>

#include <iterator>

#include "my_decompose.hh"
#include "json_helper.hh" // IWYU pragma: keep

using namespace Qt::Literals;

void from_json(const nlohmann::json &j, MyDecomposeData::ListItem &item)
{
    j.at("act_name").get_to(item.act_name);
    j.at("act_id").get_to(item.act_id);
    j.at("card_num").get_to(item.card_num);
}

void from_json(const nlohmann::json &j, MyDecomposeData &data)
{
    data.list.reset();
    auto &&list = j.at("list");
    if (list.is_array()) {
        list.get_to(data.list.emplace());
    }
}

MyDecomposeData MyDecomposeData::fromJson(const QByteArray &json, bool *ok)
{
    if (ok) {
        *ok = false;
    }

    MyDecomposeData d;

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

MyDecompose::MyDecompose(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      table_widget_(new QTableWidget(this)),
      refresh_button_(new QPushButton(u"刷新"_s, this)),
      export_button_(new QPushButton(u"导出"_s, this))
{
    refresh_button_->adjustSize();
    export_button_->adjustSize();
    table_widget_->setColumnCount(5);
    table_widget_->setHorizontalHeaderLabels(QStringList{
            u"收藏集名称"_s,
            u"Activity ID"_s,
            u"卡片数量"_s,
            u"卡片种类数"_s,
            u"操作"_s,
    });

    connect(refresh_button_, &QPushButton::clicked, this, &MyDecompose::refreshRequested);
    connect(export_button_, &QPushButton::clicked, this, &MyDecompose::exportRequested);
}

void MyDecompose::clearMyDecomposeData()
{
    table_widget_->setRowCount(0);
    map_.clear();
    // disable sorting before setting data
    table_widget_->setSortingEnabled(false);
}

void MyDecompose::setMyDecomposeData(int scene, const MyDecomposeData &data)
{
    Q_ASSERT(scene == 1 || scene == 2);

    if (!data.list.has_value()) {
        return;
    }

    // 我们需要存储卡片数量/种类数，重载比较运算符即可
    class MyWidgetItem : public QTableWidgetItem
    {
        using QTableWidgetItem::QTableWidgetItem;
        bool operator<(const QTableWidgetItem &other) const override
        {
            return text().toInt() < other.text().toInt();
        }
    };

    if (table_widget_->rowCount() == 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions)
        table_widget_->setRowCount(std::size(data.list.value()));
        for (int i = 0; i < static_cast<int>(std::size(data.list.value())); ++i) {
            auto act_name = new QLabel(
                    u"<a href=\"https://www.bilibili.com/h5/mall/digital-card/home?-Abrowser=live&act_id=%1&hybrid_set_header=2\">%2</a>"_s
                            .arg(data.list->at(i).act_id)
                            .arg(data.list->at(i).act_name));
            act_name->setTextInteractionFlags(Qt::TextBrowserInteraction);
            act_name->setOpenExternalLinks(true);
            table_widget_->setCellWidget(i, 0, act_name);
            table_widget_->setItem(i, 1,
                                   new QTableWidgetItem(QString::number(data.list->at(i).act_id)));
            table_widget_->setItem(i, scene == 1 ? 2 : 3,
                                   new MyWidgetItem(QString::number(data.list->at(i).card_num)));
            auto detail_button = new QPushButton(u"详细"_s);
            connect(detail_button, &QPushButton::clicked, this,
                    [this, act_id = data.list->at(i).act_id,
                     act_name = data.list->at(i).act_name]() {
                        emit detailRequested(act_id, act_name);
                    });
            table_widget_->setCellWidget(i, 4, detail_button);
            map_[data.list->at(i).act_id] = i;
        }
    } else {
        for (int i = 0; i < static_cast<int>(std::size(data.list.value())); ++i) {
            auto iter = map_.constFind(data.list->at(i).act_id);
            if (iter == map_.constEnd()) {
                qWarning() << "Unknown act_id:" << data.list->at(i).act_id;
                continue;
            }
            if (QTableWidgetItem *item = table_widget_->item(iter.value(), scene == 1 ? 2 : 3)) {
                Q_ASSERT(dynamic_cast<MyWidgetItem *>(item) != nullptr);
                item->setText(QString::number(data.list->at(i).card_num));
            } else {
                table_widget_->setItem(
                        iter.value(), scene == 1 ? 2 : 3,
                        new MyWidgetItem(QString::number(data.list->at(i).card_num)));
            }
        }
        // enable sorting after setting all data
        table_widget_->setSortingEnabled(true);
        // `map_` should not be used from now on...
    }
    table_widget_->resizeColumnsToContents();
}

void MyDecompose::disableExportButton()
{
    export_button_->setDisabled(true);
}

void MyDecompose::enableExportButton()
{
    export_button_->setEnabled(true);
}

void MyDecompose::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const QSize size = event->size();
    table_widget_->resize(size.width(),
                          size.height() - 2
                                  - qMax(refresh_button_->height(), export_button_->height()));
    refresh_button_->move(table_widget_->geometry().bottomLeft() + QPoint(0, 1));
    export_button_->move(refresh_button_->geometry().topRight() + QPoint(5, 0));
}