#ifndef MY_DECOMPOSE_HH
#define MY_DECOMPOSE_HH

#include <QWidget>
#include <QList>
#include <QMap>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QPushButton;
QT_END_NAMESPACE

struct MyDecomposeData
{
    QString act_name;
    int act_id;
    int card_num;
};

class MyDecompose : public QWidget
{
    Q_OBJECT

public:
    explicit MyDecompose(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

signals:
    void refreshRequested();
    void detailRequested(int act_id, const QString &act_name);

public slots:
    void clearMyDecomposeData();
    void setMyDecomposeData(int scene, const QList<MyDecomposeData> &data);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QTableWidget *table_widget_;
    QPushButton *refresh_button_;
    QMap<int, int> map_; // {act_id, row_id}
};

#endif