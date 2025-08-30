#include <QNetworkAccessManager>
#include <QHttpHeaders>
#include <QUrl>
#include <QUrlQuery>
#include <QScopedPointer>
#include <QtLogging>
#include <QDebug>
#include <QtAssert>

#include "bilibili_request_manager.hh"
#include "compress_helper.hh"

using namespace Qt::Literals;

BilibiliRequestManager::BilibiliRequestManager(QObject *parent)
    : QObject(parent),
      manager_(new QNetworkAccessManager(this)),
      factory_(QUrl(u"https://api.bilibili.com"_s)),
      user_agent_(u"Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
                  "AppleWebKit/537.36 (KHTML, like Gecko) "
                  "Chrome/139.0.0.0 "
                  "Safari/537.36"_s)
{
    // 默认构造不会配置 cookie，使用默认的 UA
    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::AcceptEncoding, "gzip, deflate, br"_L1);
    headers.append(QHttpHeaders::WellKnownHeader::UserAgent, user_agent_);
    factory_.setCommonHeaders(headers);

    connect(manager_, &QNetworkAccessManager::sslErrors, this, &BilibiliRequestManager::sslErrors);
}

void BilibiliRequestManager::setUserAgent(const QString &user_agent)
{
    user_agent_ = user_agent;

    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::AcceptEncoding, "gzip, deflate, br"_L1);
    headers.append(QHttpHeaders::WellKnownHeader::UserAgent, user_agent);
    headers.append(QHttpHeaders::WellKnownHeader::Cookie, cookie_);
    factory_.setCommonHeaders(headers);
}

void BilibiliRequestManager::setCookie(const QString &cookie)
{
    cookie_ = cookie;
    bool ok = false;
    for (auto &&kv_pair : cookie.toLatin1().split(';')) {
        const auto kv = kv_pair.split('=');

        if (std::size(kv) != 2) {
            qWarning() << "Invalid (key, value):" << kv_pair;
            continue;
        }

        if (kv[0].trimmed() == "bili_jct") {
            csrf_ = QString::fromLatin1(kv[1].trimmed());
            ok = true;
        }

        if (kv[0].trimmed() == "buvid3") {
            buvid_ = QString::fromLatin1(kv[1].trimmed());
        }
    }

    if (!ok) {
        qWarning() << "Invalid cookie:" << cookie;
    }

    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::AcceptEncoding, "gzip, deflate, br"_L1);
    headers.append(QHttpHeaders::WellKnownHeader::UserAgent, user_agent_);
    headers.append(QHttpHeaders::WellKnownHeader::Cookie, cookie);
    factory_.setCommonHeaders(headers);
}

void BilibiliRequestManager::getMyDecompose(int scene)
{
    QNetworkRequest request = factory_.createRequest(u"/x/vas/smelt/my_decompose/info"_s,
                                                     QUrlQuery{
                                                             { u"csrf"_s, csrf_ },
                                                             { u"scene"_s, QString::number(scene) },
                                                     });
    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError error) {
        emit errorOccurred(qobject_cast<QNetworkReply *>(sender()), error);
    });
    connect(reply, &QNetworkReply::finished, this, [this, scene]() {
        QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply(
                qobject_cast<QNetworkReply *>(sender()));
        Q_ASSERT(reply != nullptr);

        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        const QByteArray data = reply->readAll();
        const QHttpHeaders headers = reply->headers();

        if (headers.contains(QHttpHeaders::WellKnownHeader::ContentEncoding)) {
            bool ok;
            const QByteArray uncompressed_data = uncompress(
                    data, headers.value(QHttpHeaders::WellKnownHeader::ContentEncoding), &ok);
            if (ok) {
                emit myDecomposeDataReceived(scene, uncompressed_data);
            } else {
                qWarning() << "Unexpected Content-Encoding:"
                           << headers.value(QHttpHeaders::WellKnownHeader::ContentEncoding);
            }
        } else {
            emit myDecomposeDataReceived(scene, data);
        }
    });
}

void BilibiliRequestManager::getAssetBag(int act_id, const QString &act_name, int lottery_id,
                                         int ruid)
{
    QNetworkRequest request =
            factory_.createRequest(u"/x/vas/dlc_act/asset_bag"_s,
                                   QUrlQuery{
                                           { u"act_id"_s, QString::number(act_id) },
                                           { u"buvid"_s, buvid_ },
                                           { u"csrf"_s, csrf_ },
                                           { u"lottery_id"_s, QString::number(lottery_id) },
                                           { u"ruid"_s, QString::number(ruid) },
                                   });
    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError error) {
        emit errorOccurred(qobject_cast<QNetworkReply *>(sender()), error);
    });
    connect(reply, &QNetworkReply::finished, this, [this, act_id, act_name, lottery_id, ruid]() {
        QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply(
                qobject_cast<QNetworkReply *>(sender()));
        Q_ASSERT(reply != nullptr);

        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        const QByteArray data = reply->readAll();
        const QHttpHeaders headers = reply->headers();

        if (headers.contains(QHttpHeaders::WellKnownHeader::ContentEncoding)) {
            bool ok;
            const QByteArray uncompressed_data = uncompress(
                    data, headers.value(QHttpHeaders::WellKnownHeader::ContentEncoding), &ok);
            if (ok) {
                emit assetBagDataReceived(act_id, act_name, lottery_id, ruid, uncompressed_data);
            } else {
                qWarning() << "Unexpected Content-Encoding:"
                           << headers.value(QHttpHeaders::WellKnownHeader::ContentEncoding);
            }
        } else {
            emit assetBagDataReceived(act_id, act_name, lottery_id, ruid, data);
        }
    });
}

void BilibiliRequestManager::getImage(long long card_type_id, const QUrl &url)
{
    QNetworkRequest request(url);
    {
        // 不需要 cookie
        QHttpHeaders headers;
        // 传输图片不接受压缩后的数据，因为主流图片格式本身已经是压缩后的结果
        headers.append(QHttpHeaders::WellKnownHeader::UserAgent, user_agent_);
        request.setHeaders(headers);
    }
    QNetworkReply *reply = manager_->get(request);
    connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError error) {
        emit errorOccurred(qobject_cast<QNetworkReply *>(sender()), error);
    });
    connect(reply, &QNetworkReply::finished, this, [this, card_type_id, url]() {
        QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply(
                qobject_cast<QNetworkReply *>(sender()));
        Q_ASSERT(reply != nullptr);

        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        emit imageDataReceived(card_type_id, url, reply->readAll());
    });
}