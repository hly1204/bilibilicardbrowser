#ifndef MY_FAVORITE_HH
#define MY_FAVORITE_HH

#include <QWidget>

class MyFavorite : public QWidget
{
    Q_OBJECT

public:
    explicit MyFavorite(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
};

#endif