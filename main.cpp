#include "mainwindow.h"
#include <QApplication>
#include <QPalette>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    {
    #if 1
        QPalette palette;
        w.setAutoFillBackground(true);
        QPixmap pixmap(":/icon/image/bg-main6.png");
        palette.setBrush(QPalette::Background, QBrush(pixmap));
        w.setPalette(palette);
    #endif

    #if 0
        QGraphicsScene *scene = new QGraphicsScene;
        QGraphicsProxyWidget *s = scene->addWidget(&w);
        s->setRotation(90);
        QGraphicsView *view = new QGraphicsView(scene);
        view->resize(800, 480);
        view->show();
    #else
        w.show();
    #endif
    }

    return a.exec();
}
