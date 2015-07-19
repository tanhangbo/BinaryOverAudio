#include "dialog.h"
#include "ui_dialog.h"
#include <QByteArray>
#include <QDebug>
#include "global.h"


//data+nav - ack
//backoff window
//1-10
//frame format:
//head(0xAA)1B+head(mac 4B)+datalen+ fcs(head and data crc)
//

QMutex myMutex;


unsigned char ringBuffer[RING_SIZE];
int ringEnd=0;
int ringStart=0;
int used=0;

QList<unsigned char> recvList;



void Dialog::processTheDatagram(QByteArray datagram)
{

    emit sendData(datagram);
    return;

    //写文件子程序
    char *p=new char[datagram.size()+1];
    memset(p,0,datagram.size());

    for (int i = 1; i < datagram.size(); i+=2)
    {


        if(datagram.at(i)>-50 && datagram.at(i)<50)
        {
            QString str=QString::number(0)+",";
            file->write(str.toLatin1());
        }
        else
        {
            QString str=QString::number(1)+",";
            file->write(str.toLatin1());
        }


        //qDebug("%d-",(signed char)datagram.at(i));
    }


    file->write("OVER");
    return;





    //波形显示子程序
    int isShow=0;
    if(isShow)
    {
        scene->clear();
        int x=0;
        for (int i = 0; i +1 < datagram.size(); i+=2) {
            //qDebug()<<(unsigned char)datagram.at(i+1);
            //scene->addLine(x,256,x,128-(signed char)datagram.at(i+1));
            x++;
        }
    }

}

void Dialog::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 1234);//reveive

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}

void Dialog::readPendingDatagrams()
{
    //qDebug()<<"!";
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        processTheDatagram(datagram);
    }
}



void Dialog::haveBinary(QByteArray arr)
{

    for(int i=0;i<arr.size();i++)
    {

        //process data stream
        myMutex.lock();
        recvList.push_back((unsigned char)arr.at(i));
        myMutex.unlock();
        ui->log->moveCursor (QTextCursor::End);
        ui->log->insertPlainText(QString::number(arr.at(i))+",");
        ui->log->moveCursor (QTextCursor::End);

        //ui->you->appendPlainText();
    }


    /*   arr1.append(arr);
    if(arr1.size()>30){//不换行
        QString str;
        for(int i=0;i<arr1.size();i++)
            str.append(QString::number(arr1.at(i))+",");
         ui->you->appendPlainText(str);
         arr1.clear();
    }
   */
}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    initSocket();
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    premable = 0xAA;
    file = new QFile();
    file->setFileName("1.txt");
    file->open(QIODevice::WriteOnly | QIODevice::Text);
    file->reset();
    thread1 = new myThread;
    thread1->start();
    udpSocketOut = new QUdpSocket(this);

    connect(this,SIGNAL(sendData(QByteArray)),thread1,SLOT(getData(QByteArray)));
    connect(thread1,SIGNAL(getByte(QByteArray)),this,SLOT(haveBinary(QByteArray)));
    connect(thread1,SIGNAL(getRealData(QByteArray,int)),this,SLOT(recvRealData(QByteArray,int)));
}



void Dialog::recvRealData(QByteArray b, int MAC)
{
    ui->you->appendPlainText("("+QString::number(MAC)+")  say:"+QString(b));
}

Dialog::~Dialog()
{
    file->close();
    delete ui;
}


void Dialog::sendToSpeaker(char*data,int totalLen)//send via UDP
{

    QByteArray datagram(data,totalLen);
    udpSocketOut->writeDatagram(datagram.data(), datagram.size(), QHostAddress::LocalHost, 1500);
}
void Dialog::formFrame(QString str)
{

    //frame format:
    // PREMABLE  MAC_ADDR DATASIZE      DATA       CRC
    //          1                4                4           DATASIZE      1

    //3bit/s at least 11*8/3 = 29s
    //for 10B message 20*8/3 = 53.33s ~ = 1min
    int totalLen=0;//B
    totalLen+=1;//premable 0xAA
    totalLen+=4;//MAC
    char MAC[] = {0x02,0x00,0x00,0x00};
    QByteArray text = str.toLocal8Bit();
    int dataSize=text.size();
    totalLen+=4;//size hint
    totalLen+=dataSize;
    totalLen+=1;//CRC
    char *data = new char[totalLen];

    char crc=0x33;
    memcpy(data,&premable,1);
    memcpy(data+1,MAC,4);
    memcpy(data+5,&dataSize,4);
    memcpy(data+9, text.data(),dataSize);
    memcpy(data+totalLen-1,&crc,1);//add crc
    sendToSpeaker(data,totalLen);


    //debug data output
    QString debugOut;
    for(int i=0;i<totalLen;i++)
    {
        debugOut+=QString::number(data[i],16);
        debugOut+=",";
    }
    qDebug()<<debugOut;


    delete [] data;
}

void Dialog::on_send_clicked()
{
    QString myText = ui->me->toPlainText();
    if(myText.size()==0)
        return;
    ui->you->appendPlainText("I say:"+myText);
    ui->me->clear();
    formFrame(myText);
}
