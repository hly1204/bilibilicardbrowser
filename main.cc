#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QString>

#include "main_window.hh"
#include "asset_bag.hh"
#include "my_decompose.hh"

using namespace Qt::Literals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qRegisterMetaType<AssetBagData>("AssetBagData");
    qRegisterMetaType<QList<MyDecomposeData>>("QList<MyDecomposeData>");

    app.setApplicationName(u"我的小卡片"_s); // NOLINT(readability-static-accessed-through-instance)
    app.setApplicationVersion( // NOLINT(readability-static-accessed-through-instance)
            APPLICATION_VERSION);

    QTranslator t;
    if (t.load(QLocale(QLocale::Chinese), u"qt"_s, u"_"_s,
               QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&t); // NOLINT(readability-static-accessed-through-instance)
    } else {
        qWarning() << "Failed to load translations";
    }

    MainWindow win;
    win.show();

    return app.exec(); // NOLINT(readability-static-accessed-through-instance)
}