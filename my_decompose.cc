#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QResizeEvent>
#include <QtLogging>
#include <QDebug>
#include <QtAssert>

#include <iterator>

#include "my_decompose.hh"

using namespace Qt::Literals;

MyDecompose::MyDecompose(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      table_widget_(new QTableWidget(this)),
      refresh_button_(new QPushButton(u"刷新"_s, this))
{
    refresh_button_->adjustSize();
    table_widget_->setColumnCount(5);
    table_widget_->setHorizontalHeaderLabels(QStringList{
            u"收藏集名称"_s,
            u"Activity ID"_s,
            u"卡片数量"_s,
            u"卡片种类数"_s,
            u"操作"_s,
    });

    connect(refresh_button_, &QPushButton::clicked, this, &MyDecompose::refreshRequested);
}

void MyDecompose::clearMyDecomposeData()
{
    table_widget_->setRowCount(0);
    map_.clear();
}

void MyDecompose::setMyDecomposeData(int scene, const QList<MyDecomposeData> &data)
{
    Q_ASSERT(scene == 1 || scene == 2);

    if (table_widget_->rowCount() == 0) {
        table_widget_->setRowCount(
                std::size(data)); // NOLINT(cppcoreguidelines-narrowing-conversions)
        for (int i = 0; i < static_cast<int>(std::size(data)); ++i) {
            QLabel *act_name = new QLabel(
                    u"<a href=\"https://www.bilibili.com/h5/mall/digital-card/home?-Abrowser=live&act_id=%1&hybrid_set_header=2\">%2</a>"_s
                            .arg(data[i].act_id)
                            .arg(data[i].act_name));
            act_name->setTextInteractionFlags(Qt::TextBrowserInteraction);
            act_name->setOpenExternalLinks(true);
            table_widget_->setCellWidget(i, 0, act_name);
            table_widget_->setItem(i, 1, new QTableWidgetItem(QString::number(data[i].act_id)));
            table_widget_->setItem(i, scene == 1 ? 2 : 3,
                                   new QTableWidgetItem(QString::number(data[i].card_num)));
            QPushButton *detail_button = new QPushButton(u"详细"_s);
            connect(detail_button, &QPushButton::clicked, this,
                    [this, act_id = data[i].act_id, act_name = data[i].act_name]() {
                        emit detailRequested(act_id, act_name);
                    });
            table_widget_->setCellWidget(i, 4, detail_button);
            map_[data[i].act_id] = i;
        }
    } else {
        for (int i = 0; i < static_cast<int>(std::size(data)); ++i) {
            auto iter = map_.constFind(data[i].act_id);
            if (iter == map_.constEnd()) {
                qWarning() << "Unknown act_id:" << data[i].act_id;
                continue;
            }
            if (QTableWidgetItem *item = table_widget_->item(iter.value(), scene == 1 ? 2 : 3)) {
                item->setText(QString::number(data[i].card_num));
            } else {
                table_widget_->setItem(iter.value(), scene == 1 ? 2 : 3,
                                       new QTableWidgetItem(QString::number(data[i].card_num)));
            }
        }
    }
    table_widget_->resizeColumnsToContents();
}

void MyDecompose::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const QSize size = event->size();
    table_widget_->resize(size.width(), size.height() - 2 - refresh_button_->height());
    refresh_button_->move(table_widget_->geometry().bottomLeft() + QPoint(0, 1));
}