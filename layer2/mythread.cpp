#include "mythread.h"
#include "global.h"
#include <QList>
myThread::myThread(QObject *parent) :
    QThread(parent)
{
}




int count =0;


int getn(int num)
{
    int total=0;
    for(int i=0;i<num;i++)
    {
        if(ringStart!=RING_SIZE)
            total+=ringBuffer[ringStart++];
        else
        {
            ringStart=0;
            total+=ringBuffer[ringStart++];
        }
        used--;
    }
    //qDebug()<<num<<":"<<total;
    return total;
}


void myThread::getData(QByteArray datagram)
{

    for(int i=1;i<datagram.size();i+=2)//write
    {
        unsigned char now=1;
        if(datagram.at(i)>-30 && datagram.at(i)<30)//抗干扰系数
            now=0;

        if(ringEnd!=RING_SIZE)
            ringBuffer[ringEnd++] = now;
        else
        {
            ringEnd=0;
            ringBuffer[ringEnd++] = now;
        }
        used++;
    }
    if(used==RING_SIZE)//buffer is full!
        assert(0);




    /*      测试代码
        while(used>100)
        {
            int oldCount = count;
            int have=0;
            while(1)
            {
                if(getn(50)>10)
                {
                    count++;
                    have=1;
                    continue;
                }
                break;
            }

            int diff=count-oldCount;
            if(have && diff>4)
                if(diff>20)
                    qDebug()<<"1";
           else
                    qDebug()<<"0";

        }
        return;
    */


    //start process
    QByteArray myDat;
    int bigsize=2000;
    while(used>bigsize)
    {
        while(getn(10)<3)
        {
            if(used<bigsize)
            {
                break;
            }
        }

        // qDebug()<<"goinbig";
        if(used> bigsize)
        {
            int getSize=getn(bigsize);
            //qDebug()<<getSize;
            if(getSize>500)
            {
                //qDebug()<<"1";
                myDat.append((char)1);
            }
            else if(getSize>100)
            {
                //qDebug()<<"0";
                myDat.append((char)0);
            }
        }


    }


    if(myDat.length()>0)
        emit getByte(myDat);

}

char parseByte( QList<unsigned char>& list1)
{
    char byte=0x00;
    for(int i=0;i<8;i++)
    {
        if(list1.front() == 0x01)
            byte = byte | (1<<i);
        list1.pop_front();
    }
    return byte;

}

enum rcvState{
    STATE_DETECT_HEADER = 0,
    STATE_RECV_MAC,
    STATE_RECV_LEN,
    STATE_RECV_DATA,
    STATE_RECV_FCS,
    STATE_CHECK_ACK
};

void  myThread::run(){
    QList<unsigned char> list1;
    int state = STATE_DETECT_HEADER;
    int len=0;
    int MAC=0;
    char Fheader=0x00;
    char premable = 0xAA;
    bool isValid;
    QByteArray msg;

    char FMAC0;
    char FMAC1;
    char FMAC2;
    char FMAC3;

    char  FLen0;
    char  FLen1;
    char  FLen2;
    char  FLen3;


    while(1)
    {

        myMutex.lock();
        if(!recvList.empty())
        {
            list1.push_back(recvList.first());
            recvList.pop_front();
        }
        myMutex.unlock();


        //now we have got a lot of data

        switch(state)
        {

        case STATE_DETECT_HEADER:
            msg.clear();
            FMAC0=0x00;
            FMAC1=0x00;
            FMAC2=0x00;
            FMAC3=0x00;

            FLen0=0x00;
            FLen1=0x00;
            FLen2=0x00;
            FLen3=0x00;

            len=0;
            MAC=0;

            if(list1.length()<8)
                break;

            isValid = true;
            while(list1.length()>=8)
            {
                for(int i=0;i<8;i++)//little endian
                {
                    if(list1.at(i)!= ((premable>>i)&0x01))
                    {
                        isValid = false;
                        break;
                    }
                    qDebug()<<i;
                }
                if(!isValid)
                    list1.pop_front();

                break;
            }
            if(!isValid)
                break;


            /*******frame format*******
                PREMABLE  MAC_ADDR DATASIZE      DATA       CRC
                          1                4                4           DATASIZE      1
                ****************************/

            //read header
            Fheader = parseByte(list1);
            qDebug()<<"We have got valid header"<<QString("%1").arg(Fheader,0,16);
            state = STATE_RECV_MAC;
            break;

        case STATE_RECV_MAC:
            if(list1.length()<32)
                break;

            //read MAC
            FMAC0 =  parseByte(list1);
            FMAC1 =  parseByte(list1);
            FMAC2 =  parseByte(list1);
            FMAC3 =  parseByte(list1);
            MAC |= FMAC0;
            MAC |= FMAC1<<8;
            MAC |= FMAC2<<16;
            MAC |= FMAC3<<24;
            qDebug()<<"We have got valid MAC"<<QString("%1").arg(MAC,0,16);
            state = STATE_RECV_LEN;
            break;
        case STATE_RECV_LEN:
            if(list1.length()<32)
                break;
            FLen0 = parseByte(list1);
            FLen1 = parseByte(list1);
            FLen2 = parseByte(list1);
            FLen3 = parseByte(list1);

            len = len | FLen0;
            len = len | FLen1<<8;
            len = len | FLen2<<16;
            len = len | FLen3<<24;
            qDebug()<<"We have got valid LEN"<<len;
            state = STATE_RECV_DATA;
            if(len ==0 || len >100)
                state =STATE_DETECT_HEADER;
            break;

        case STATE_RECV_DATA:
            if(list1.length()<len*8)
                break;
            for(int i=0;i<len;i++)
                msg.append(parseByte(list1));
            qDebug()<<"got msg:"<<QString(msg);
            state = STATE_RECV_FCS;
            break;

        case STATE_RECV_FCS:

            if(list1.length()<8)
                break;

            qDebug()<<"got fcs:"<<QString("%1").arg(parseByte(list1),0,16);

            emit getRealData(msg,MAC);
            state =STATE_DETECT_HEADER;
            break;

        case STATE_CHECK_ACK:
            state = -1;
            break;

        }/*end switch state*/
        msleep(10);
    }
}
