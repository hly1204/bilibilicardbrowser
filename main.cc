#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QString>

#include "main_window.hh"

using namespace Qt::Literals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(u"我的小卡片"_s);
    app.setApplicationVersion(APPLICATION_VERSION);

    QTranslator t;
    if (t.load(QLocale(QLocale::Chinese), u"qt"_s, u"_"_s,
               QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&t);
    } else {
        qWarning() << "Failed to load translations";
    }

    MainWindow win;
    win.show();

    return app.exec();
}