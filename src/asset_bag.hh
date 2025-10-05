#ifndef ASSET_BAG_HH
#define ASSET_BAG_HH

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QUrl>
#include <QDateTime>

#include <optional>

QT_BEGIN_NAMESPACE
class QLabel;
class QTreeWidget;
class QPushButton;
QT_END_NAMESPACE

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct AssetBagData
{
    static AssetBagData fromJson(const QByteArray &json, bool *ok = nullptr);

    int total_item_cnt;
    int owned_item_cnt;

    struct CardIdListItem
    {
        long long card_id;
        QString card_no;
        int status;
        struct
        {
            int is_transfer;
        } card_right;
    };

    struct ListItem
    {
        int item_type;
        int item_scarcity;

        struct CardItem
        {
            long long card_type_id;
            QString card_name;
            QUrl card_img;
            int card_type;
            std::optional<QList<CardIdListItem>> card_id_list;
            int total_cnt;
            QString total_cnt_show;
            int holding_rate; // 除以 100 显示百分比
            int card_scarcity;
            int is_limited_card;
        };
        std::optional<CardItem> card_item;

        [[nodiscard]] inline QString scarcity() const;
    };

    struct CollectListItem
    {
        int collect_id;
        QDateTime start_time;
        QDateTime end_time;
        QString redeem_text;
        int redeem_item_type;
        QString redeem_item_id;
        QString redeem_item_name;
        QUrl redeem_item_image;
        int owned_item_amount;
        int require_item_amount;
        int has_redeemed_cnt;
        int effective_forever;

        struct CardItem
        {
            struct CardTypeInfo
            {
                long long id;
                QString name;
                QUrl overview_image;
                int scarcity;
            };
            std::optional<CardTypeInfo> card_type_info;
            std::optional<ListItem> card_asset_info;
        };
        std::optional<CardItem> card_item;
    };

    struct LotterySimpleListItem
    {
        int lottery_id;
        QString lottery_name;
    };

    std::optional<QList<ListItem>> item_list;
    std::optional<QList<CollectListItem>> collect_list;
    std::optional<QList<LotterySimpleListItem>> lottery_simple_list;
};

class AssetBag : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int actId READ actId)
    Q_PROPERTY(int lotteryId READ lotteryId)
    Q_PROPERTY(QString actName READ actName)
    Q_PROPERTY(QString lotteryName READ lotteryName)

public:
    explicit AssetBag(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    [[nodiscard]] int actId() const { return act_id_; }
    [[nodiscard]] int lotteryId() const { return lottery_id_; }
    [[nodiscard]] QString actName() const { return act_name_; }
    [[nodiscard]] QString lotteryName() const { return lottery_name_; }

signals:
    void refreshRequested(int act_id, const QString &act_name, int lottery_id,
                          const QString &lottery_name);

public slots:
    void setInfo(int act_id, int lottery_id, const QString &act_name, const QString &lottery_name);
    void setInfo(int act_id, const QString &act_name)
    {
        setInfo(act_id, 0, act_name, QStringLiteral("全部奖池"));
    }
    void clearAssetBagData();
    void setAssetBagData(const AssetBagData &data);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    int act_id_;
    int lottery_id_; ///< 若为零则为全部奖池，否则为单奖池
    QString act_name_;
    QString lottery_name_;
    QLabel *link_label_;
    QLabel *item_cnt_label_;
    QTreeWidget *tree_widget_;
    QPushButton *refresh_button_;
};

// clang-format off
inline QString AssetBagData::ListItem::scarcity() const
{
    if (!card_item.has_value()) {
        return QStringLiteral("非卡片");
    }
    switch (card_item->card_scarcity) {
    case 0: return QStringLiteral("典藏卡");
    case 10: return QStringLiteral("普卡");
    case 20: return QStringLiteral("稀缺");
    case 30: return QStringLiteral("小隐藏");
    case 40: return QStringLiteral("大隐藏");
    default: return QStringLiteral("未知");
    }
};
// clang-format on

#endif