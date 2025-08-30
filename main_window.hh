#ifndef MAIN_WINDOW_HH
#define MAIN_WINDOW_HH

#include <QMainWindow>
#include <QThread>
#include <QString>

#include "bilibili_request_manager.hh"

QT_BEGIN_NAMESPACE
class QSplitter;
class QPushButton;
QT_END_NAMESPACE

class MyDecompose;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~MainWindow() override;

private slots:
    void onSetCookieButtonClicked();
    void onMyDecomposeDataReceived(int scene, const QByteArray &json);

private:
    QThread network_thread_;
    BilibiliRequestManager manager_;
    QSplitter *splitter_;
    MyDecompose *my_decompose_;
    QPushButton *set_cookie_button_;
};

#endif