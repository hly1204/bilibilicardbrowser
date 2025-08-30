#ifndef BILIBILI_REQUEST_MANAGER_HH
#define BILIBILI_REQUEST_MANAGER_HH

#include <QObject>
#include <QByteArray>
#include <QString>
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

public:
    explicit BilibiliRequestManager(QObject *parent = nullptr);

    void setUserAgent(const QString &user_agent);
    QString userAgent() const { return user_agent_; }

    void setCookie(const QString &cookie);
    QString cookie() const { return cookie_; }
    QString csrf() const { return csrf_; }

public slots:
    void getMyDecompose(int scene);

signals:
    void myDecomposeDataReceived(int scene, const QByteArray &json);

signals:
    void errorOccurred(QNetworkReply *reply, QNetworkReply::NetworkError error);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    QNetworkAccessManager *manager_;
    QNetworkRequestFactory factory_;
    QString user_agent_;
    QString cookie_;
    QString csrf_;
};

#endif