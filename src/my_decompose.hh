#ifndef MY_DECOMPOSE_HH
#define MY_DECOMPOSE_HH

#include <QWidget>
#include <QByteArray>
#include <QList>
#include <QMap>

#include <optional>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QPushButton;
QT_END_NAMESPACE

struct MyDecomposeData
{
    static MyDecomposeData fromJson(const QByteArray &json, bool *ok = nullptr);

    struct ListItem
    {
        QString act_name;
        int act_id;
        int card_num;
    };

    std::optional<QList<ListItem>> list;
};

Q_DECLARE_METATYPE(MyDecomposeData)

class MyDecompose : public QWidget
{
    Q_OBJECT

public:
    explicit MyDecompose(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void refreshRequested();
    void exportRequested();
    void detailRequested(int act_id, const QString &act_name);
    void likeButtonClicked(int act_id, const QString &act_name);

public slots:
    void clearMyDecomposeData();
    void setMyDecomposeData(int scene, const MyDecomposeData &data);
    void disableExportButton();
    void enableExportButton();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QTableWidget *table_widget_;
    QPushButton *refresh_button_;
    QPushButton *export_button_;
    QMap<int, int> map_; // {act_id, row_id}
};

#endif