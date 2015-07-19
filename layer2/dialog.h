#ifndef DIALOG_H
#define DIALOG_H

#include <QUdpSocket>
#include "mythread.h"
#include <Qfile>
#include <QtWidgets>

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
    QUdpSocket *udpSocket;
    QUdpSocket *udpSocketOut;
    void initSocket();
    void processTheDatagram(QByteArray);
    QGraphicsScene *scene;
    void formFrame(QString);
    void sendToSpeaker(char*,int);
    char premable ;

       QFile *file;
       myThread * thread1;
signals:
           void sendData(QByteArray);
 private slots:
    void readPendingDatagrams();
    void haveBinary(QByteArray);
    void recvRealData(QByteArray,int);
    void on_send_clicked();
};

#endif // DIALOG_H
