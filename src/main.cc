#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QScopeGuard>

#include "main_window.hh"
#include "asset_bag.hh"
#include "my_decompose.hh"

using namespace Qt::Literals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<MyDecomposeData>("MyDecomposeData");
    qRegisterMetaType<AssetBagData>("AssetBagData");

    // NOLINTNEXTLINE(readability-static-accessed-through-instance)
    app.setApplicationName(u"bilibilicardbrowser"_s);
    // NOLINTNEXTLINE(readability-static-accessed-through-instance)
    app.setApplicationVersion(APPLICATION_VERSION);

    QTranslator t;
    if (t.load(QLocale(QLocale::Chinese), u"qt"_s, u"_"_s,
               QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        // NOLINTNEXTLINE(readability-static-accessed-through-instance)
        app.installTranslator(&t);
    } else {
        qWarning() << "Failed to load translations";
    }

    // NOLINTNEXTLINE(readability-static-accessed-through-instance)
    QFile lock_file(app.applicationDirPath() % "/" % app.applicationName() % ".lock");
    if (!lock_file.open(QFile::NewOnly | QFile::Truncate)) {
        QMessageBox msg_box;
        msg_box.setIcon(QMessageBox::Warning);
        msg_box.setWindowTitle(u"警告"_s);
        msg_box.setText(u"为了防止数据竞争 (Data Race) 问题，目前仅支持同时打开一个应用程序。"_s);
        msg_box.setDetailedText(
                u"该警告出现可能因为应用程序没有权限在目录中创建文件或上次未正常关闭。"
                "若因为上次未正常关闭该应用程序，则手动删除 %1 文件即可"_s.arg(
                        lock_file.fileName()));
        msg_box.show();
        // NOLINTNEXTLINE(readability-static-accessed-through-instance)
        return app.exec();
    }

    MainWindow win;
    win.show();

    const auto cleanup = qScopeGuard([&]() { lock_file.remove(); });

    // NOLINTNEXTLINE(readability-static-accessed-through-instance)
    return app.exec();
}