#if 0
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QtSerialPort/QSerialPortInfo>
QT_USE_NAMESPACE
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
                    QVBoxLayout *layout = new QVBoxLayout;
                    QString str;
                    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
                    QString s = QObject::tr("Port: ") + info.portName() + "\n"
                    + QObject::tr("Location: ") + info.systemLocation() + "\n"
                    + QObject::tr("Description: ") + info.description() + "\n"
                    + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                    + QObject::tr("Serial number: ") + info.serialNumber() + "\n"
                    + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                    + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n";
                    str += s;
                    }
                    QLabel *label = new QLabel(str);
                    layout->addWidget(label);
                    QWidget *workPage = new QWidget;
                    workPage->setLayout(layout);
                    QScrollArea area;
                    area.setWindowTitle(QObject::tr("Info about all available serial ports."));
                    area.setWidget(workPage);
                    area.show();
                    return a.exec();
}

#else
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
#endif
