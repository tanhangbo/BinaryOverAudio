#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QDebug>
class myThread : public QThread
{
    Q_OBJECT
public:
    explicit myThread(QObject *parent = 0);
    void run();
signals:
    void getByte(QByteArray);
    void getRealData(QByteArray,int);
public slots:
    void getData(QByteArray);

};

#endif // MYTHREAD_H
