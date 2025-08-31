#ifndef JSON_HELPER_HH
#define JSON_HELPER_HH

#include <QList>
#include <QString>
#include <QUrl>
#include <QDateTime>

#include <nlohmann/json.hpp>

template <typename Tp>
inline void from_json(const nlohmann::json &j, QList<Tp> &list);
// clang-format off
inline void from_json(const nlohmann::json &j, QString &s)
{ s = QString::fromStdString(j.get<std::string>()); }
inline void from_json(const nlohmann::json &j, QUrl &u)
{ u.setUrl(j.get<QString>()); }
inline void from_json(const nlohmann::json &j, QDateTime &d)
{ d = QDateTime::fromSecsSinceEpoch(j.get<qint64>()); }
// clang-format on

template <typename Tp>
inline void from_json(const nlohmann::json &j, QList<Tp> &list)
{
    list.clear();
    if (!j.is_array()) {
        return;
    }

    list.reserve(std::size(j));
    for (auto &&item : j) {
        auto &&last_item = list.emplace_back();
        item.get_to(last_item);
    }
}

#endif