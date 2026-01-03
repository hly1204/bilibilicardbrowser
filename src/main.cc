#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QString>

#include "main_window.hh"
#include "asset_bag.hh"
#include "my_decompose.hh"

using namespace Qt::Literals;

int main(int argc, char *argv[])
{
    // NOLINTBEGIN(readability-static-accessed-through-instance)
    QApplication app(argc, argv);

    qRegisterMetaType<MyDecomposeData>("MyDecomposeData");
    qRegisterMetaType<AssetBagData>("AssetBagData");

    app.setApplicationName(u"我的小卡片"_s);
    app.setApplicationVersion(APPLICATION_VERSION);

    QTranslator t;
    if (t.load(QLocale(QLocale::Chinese), u"qt"_s, u"_"_s,
               QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        // NOLINTNEXTLINE(readability-static-accessed-through-instance)
        app.installTranslator(&t);
    } else {
        qWarning() << "Failed to load translations";
    }

    MainWindow win;
    win.show();

    return app.exec();
    // NOLINTEND(readability-static-accessed-through-instance)
}
