#ifndef BILIBILI_REQUEST_MANAGER_HH
#define BILIBILI_REQUEST_MANAGER_HH

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QNetworkRequestFactory>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

class BilibiliRequestManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString userAgent READ userAgent WRITE setUserAgent)
    Q_PROPERTY(QString cookie READ cookie WRITE setCookie)
    Q_PROPERTY(QString csrf READ csrf)
    Q_PROPERTY(QString buvid READ buvid)

public:
    explicit BilibiliRequestManager(QObject *parent = nullptr);

    void setUserAgent(const QString &user_agent);
    QString userAgent() const { return user_agent_; }

    void setCookie(const QString &cookie);
    QString cookie() const { return cookie_; }
    QString csrf() const { return csrf_; }
    QString buvid() const { return buvid_; }

public slots:
    void getMyDecompose(int scene);
    void getAssetBag(int act_id, const QString &act_name) { getAssetBag(act_id, act_name, 0); }
    void getAssetBag(int act_id, const QString &act_name, int lottery_id)
    {
        getAssetBag(act_id, act_name, lottery_id, 0);
    }
    void getAssetBag(int act_id, const QString &act_name, int lottery_id, int ruid);
    void getImage(long long card_type_id, const QUrl &url);

signals:
    void myDecomposeDataReceived(int scene, const QByteArray &json);
    void assetBagDataReceived(int act_id, const QString &act_name, int lottery_id, int ruid,
                              const QByteArray &json);
    void imageDataReceived(long long card_type_id, const QUrl &url, const QByteArray &image);

signals:
    void errorOccurred(QNetworkReply *reply, QNetworkReply::NetworkError error);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    QNetworkAccessManager *manager_;
    QNetworkRequestFactory factory_;
    QString user_agent_;
    QString cookie_;
    QString csrf_;
    QString buvid_;
};

#endif