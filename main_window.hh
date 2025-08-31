#ifndef MAIN_WINDOW_HH
#define MAIN_WINDOW_HH

#include <QMainWindow>
#include <QSettings>
#include <QThread>
#include <QString>
#include <QMap>

#include "bilibili_request_manager.hh"

QT_BEGIN_NAMESPACE
class QSplitter;
class QPushButton;
class QTabWidget;
QT_END_NAMESPACE

class MyDecompose;
class AssetBag;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~MainWindow() override;

private slots:
    void loadSettings();
    void saveSettings();

private slots:
    void onSetCookieButtonClicked();
    void onMyDecomposeDataReceived(int scene, const QByteArray &json);
    void onAssetBagDataReceived(int act_id, const QString &act_name, int lottery_id, int ruid,
                                const QByteArray &json);

public:
    struct ActIdAndLotteryId
    {
        int act_id;
        int lottery_id;

        explicit inline ActIdAndLotteryId(int act_id, int lottery_id);
        friend inline bool operator<(const ActIdAndLotteryId &lhs, const ActIdAndLotteryId &rhs);
    };

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSettings settings_;
    QThread network_thread_;
    BilibiliRequestManager manager_;
    QSplitter *splitter_;
    MyDecompose *my_decompose_;
    QTabWidget *tab_widget_;
    QPushButton *set_cookie_button_;
    QMap<ActIdAndLotteryId, AssetBag *> map_;
};

// clang-format off
inline MainWindow::ActIdAndLotteryId::ActIdAndLotteryId(int act_id, int lottery_id)
    : act_id(act_id), lottery_id(lottery_id) { }
inline bool operator<(const MainWindow::ActIdAndLotteryId &lhs,
                      const MainWindow::ActIdAndLotteryId &rhs)
{ if (lhs.act_id == rhs.act_id) { return lhs.lottery_id < rhs.lottery_id; }
  return lhs.act_id < rhs.act_id; }
// clang-format on

#endif