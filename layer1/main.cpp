#include "myMain.h"
#include <cstdlib>
#include <iostream>
#include "windows.h"
#pragma comment(lib,"wsock32.lib")

#include "math.h"
#include "process.h"
#include <list>
#include "myBuffer.h"

#define P(x) cout<<x<<endl;
#define MAXWAVEBLOCK 10 //temp define
#define MAXWAVEBUFFER 10000 //temp define

using namespace std;

HANDLE m_mutex;  //mutex

SOCKET socket_out_socket;//　
struct sockaddr_in socket_out_addr;//
SOCKET socket_in_socket;
struct sockaddr_in socket_in_addr;
list<myBuffer*> bufList;


//global
HWAVEOUT hWaveOut;
HWAVEIN hWaveIn;  

bool m_Opened;
WAVEHDR WaveHdrOut[MAXWAVEBLOCK];  
//局部变量  ?
static WAVEHDR WaveHdrIn[MAXWAVEBLOCK];  
//static char WaveOutBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//保存播音数据用内存块  
static char WaveInBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//记录录音数据用内存块  
static char WaveOutBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//保存播音数据用内存块  


   const int nplay=500;
   //gen n
   int genNum=5;
   char a[nplay];
   char b[nplay*2];
   char *all;


int enableSend;

void dump(char* buffer,long buf_len)
{
     cout<<"len"<<buf_len<<endl;
     for(long i=1;i<buf_len;i+=2)
              cout<<int(buffer[i])<<' ';
}


void socket_send(char* buffer,long buf_len)
{
     
    int len =sizeof(socket_out_addr);
    if(sendto(socket_out_socket,buffer,buf_len,0,(struct sockaddr*)&socket_out_addr,len)==SOCKET_ERROR)
    {
       closesocket(socket_out_socket);
       assert(0);
    }  
}
 
  
int DataBlock = 0;   
int MaxDataBlock = MAXWAVEBLOCK;   
int CurrentBlock = 0;    
 int canPlay=1;
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, WORD dwInstance,  
       WAVEHDR* dwParam1, DWORD dwParam2)  
{  
 switch(uMsg)  
 {//判断回调类型  
 case WOM_OPEN:  
  //播音设备正常打开时，触发该事件  
  P("o-open");
  //MessageBox(NULL,"Open success",NULL,MB_OK);  
  break;  
 case WOM_CLOSE:  
  //播音设备正常关闭时，触发该事件  
  //MessageBox(NULL,"Close success",NULL,MB_OK);  
  P("o-close");
  break;  
 case WOM_DONE:  
  //内存播放完成时，触发该事件  
  if(!waveOutUnprepareHeader(hwo, dwParam1, sizeof(WAVEHDR)))  
  {  
   //delete dwParam1->lpData;//删除语音数据内存  
   //delete dwParam1;//删除结构定义内存  
   //m_pChange->DelDataBlock();//减少内存数量记数 
   
      //canPlay=1;
    DataBlock--;
   cout<<"-O-";
  }  
  break;  
 }  
};  
 
 

 
void play(char* buf,int RecieveLen)
{
   //  while(!canPlay)
   //              Sleep(1);
                 
    if(DataBlock < MaxDataBlock)   
    {//总播音数据块符合要求   
        //char* buff1 = new char[RecieveLen];//申请播音数据内存   
        memcpy(WaveHdrOut[CurrentBlock].lpData, buf, RecieveLen);//复制语音数据   
   
        //WaveHdrOut = new WAVEHDR;//申请结构内存   
        //为结构赋值   
        WaveHdrOut[CurrentBlock].dwBufferLength=RecieveLen;   
        WaveHdrOut[CurrentBlock].dwFlags=0;   
        WaveHdrOut[CurrentBlock].dwUser=0;   
        //准备、锁定内存块   
        waveOutPrepareHeader(hWaveOut, &WaveHdrOut[CurrentBlock], sizeof(WAVEHDR));   
        //播放该语音数据   
        waveOutWrite(hWaveOut, &WaveHdrOut[CurrentBlock], sizeof(WAVEHDR));   
        DataBlock++;   
        CurrentBlock = (CurrentBlock+1) % MAXWAVEBLOCK;   
        MaxDataBlock = MAXWAVEBLOCK;   
    }else   
    {//总内存块过多，需要减少（到2个）内存块   
     //该设置保证数据最多为 MAXWAVEBLOCK个，最少为2个   
     //在当前设置下，放音最大延时为 0.5秒（忽略网络延时）。   
        MaxDataBlock = 3;   
    }   
    
    
    //canPlay=0;
}
 
 

//HWI:Handle to the waveform-audio device associated with the callback function.
//
static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, WORD dwInstance,  
       WAVEHDR* dwParam1, DWORD dwParam2)  
{  
 long result;  
 char buffer[MAXWAVEBUFFER];  
 long buf_len;  
  
 switch(uMsg)  
 {//判断回调类型  
 case WIM_OPEN:  
  //录音设备正常打开时，触发该事件  
   P("Opened");
  //MessageBox(NULL,"Open success", "提示", MB_OK);  
  break;  
 case WIM_CLOSE:  
  //录音设备正常关闭时，触发该事件  
  P("Closed");
  //MessageBox(NULL,"Close success", "提示", MB_OK);  
  break;  
 case WIM_DATA:  
  //录音设备返回数据时，触发该事件  
  //P("Data coming!"); 
  /*
    cleans up the preparation performed by the waveInPrepareHeader function.
    This function must be called after the device driver fills a buffer and
    returns it to the application. You must call this function before
    freeing the buffer.
  */
  if(!waveInUnprepareHeader(hwi, dwParam1,sizeof(WAVEHDR)))  //if ok
  {
                                 
   //P("freeing memory");
   //可以释放内存。有的情况虽然返回数据，但不能释放内存，  
   //如使用同一个WAVEHDR增加多个内存块时，会出现上述情况  
   memcpy(buffer, dwParam1->lpData , MAXWAVEBUFFER);//复制返回数据  
   buf_len = dwParam1->dwBytesRecorded;//记录录音数据长度  
   
   /*The lpData, dwBufferLength, and dwFlags members must be set before
    calling the waveInPrepareHeader or waveOutPrepareHeader function. 
    (For either function, the dwFlags member must be set to zero.)
    */
   dwParam1->dwUser=0;//恢复设置  
   dwParam1->dwBufferLength=MAXWAVEBUFFER;  
   dwParam1->dwFlags=0;  
   if(m_Opened)  
   {//录音仍在继续  
     //P("flow coming");
     result=waveInPrepareHeader(hwi,dwParam1,sizeof(WAVEHDR));  
     assert(!result);
     result=waveInAddBuffer(hwi,dwParam1,sizeof(WAVEHDR));  
     assert(!result);
   }  
  }  
  if(buf_len > 0 && buf_len <= MAXWAVEBUFFER)  
  {//有录音数据  
      //P("has data");
      //cout<<"in";
      socket_send(buffer,buf_len);
      //dump(buffer,buf_len);
      
   //发送录音数据  
   //m_pChange->SendTo(buffer, buf_len, RemotePort, LPCTSTR(RemoteHost));  
  }  
  break;  
 }  
};  



void socket_prepare()
{               

  WSADATA wsaData;
  int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if(iResult != NO_ERROR)
  printf("Error at WSAStartup()\n");
  socket_out_addr.sin_family=AF_INET;
  socket_out_addr.sin_addr.s_addr=inet_addr("127.0.0.1"); ///server的地址 
  socket_out_addr.sin_port=htons(1234); ///server的监听端口
  socket_out_socket=socket(AF_INET,SOCK_DGRAM,0);

}






//线程函数 
unsigned int __stdcall UDPThread(void *pParam) 
{ 
    char *pMsg = (char *)pParam; 
    if (!pMsg) 
    { 
        return (unsigned int)1; 
    }  
 
 
    printf("*socket server starts*\n");
    SOCKET socket_in_socket;
    struct sockaddr_in socket_in_addr;
    struct sockaddr_in from;
    int fromlen =sizeof(from);
    socket_in_addr.sin_family=AF_INET;
    socket_in_addr.sin_port=htons(1500); ///监听端口
    socket_in_addr.sin_addr.s_addr=inet_addr("127.0.0.1");; ///本机
    socket_in_socket=socket(AF_INET,SOCK_DGRAM,0);
    bind(socket_in_socket,(struct sockaddr*)&socket_in_addr,sizeof(socket_in_addr));
    while (1)
    {
          char buffer[1024]="\0";
          if (recvfrom(socket_in_socket,buffer,sizeof(buffer),0,(struct sockaddr*)&socket_in_addr,&fromlen)!=SOCKET_ERROR)
          {
             printf("Received datagram,about to send..\n");
             int len = *(int*)(buffer+5);//frame format
             for(int i=0;i<len+10;i++)
                     printf("%02x,",buffer[i]);
             printf("\n");
             if(len>0)//assert datasize>0
             {
                  cout<<"get socket buffer , enqueue"<<endl;  
                  myBuffer *buf1 = new myBuffer(len+10,buffer);
                  WaitForSingleObject(m_mutex, INFINITE); 
                  //enqueue
                  bufList.push_back(buf1);
                  ::ReleaseMutex(m_mutex);  
                  enableSend = 1;
             }            
          }
    }
 
    return (unsigned int)0; 
}  



//线程函数 
unsigned int __stdcall HardwareThread(void *pParam) 
{ 
    char *pMsg = (char *)pParam; 
    if (!pMsg) 
    { 
        return (unsigned int)1; 
    }  
    printf("*hardware writter starts*\n");
    while(true)
    {
        if(enableSend)//after received ack or initial
        {
            cout<<"start to write hardware"<<endl;    
            myBuffer *buf1 = new myBuffer();      
            int haveData=0;
            WaitForSingleObject(m_mutex, INFINITE);     
            if(!bufList.empty())//dequeue buffer
            { 
                 haveData =1;
                 cout<<bufList.size()<<endl;;
                 //buf1.copyFrom(bufList.front());
                 buf1 = bufList.front();
                 bufList.pop_front();
            }
            ::ReleaseMutex(m_mutex);  
            enableSend = 0;
            if(!haveData)
              continue;
            
            
            
            //beep starts:
            
            int outLen;
            unsigned char *p;
            p=buf1->getBuf(&outLen);
            for(int i=0;i<outLen;i++)
            {
                    printf("%02x,",p[i]);
                    for(int j=0;j<8;j++)//little endian
                    {
                       if(((p[i])>>j)&0x01)
                          play(all,nplay*genNum);    
                       else
                          play(a,sizeof(a)); 
                       Sleep(300);
                    }
            }

            
            delete buf1;
            
           
        }
        Sleep(100);
    }

    return (unsigned int)0; 
}  

  HANDLE hThread1, hThread2; 
  unsigned int uiThreadId1, uiThreadId2;  
  char *pMsg1 = "UDP receive thread."; 
  char *pMsg2 = "buffer write thread.";  
    
    


int main(int argc, char *argv[])
{
    m_mutex = ::CreateMutex(NULL, FALSE, NULL); 
    enableSend = 0;
    socket_prepare();
    
    /*
    TWaveFormatEx 结构:
    TWaveFormatEx = packed record
    wFormatTag: Word;       {指定格式类型; 默认 WAVE_FORMAT_PCM = 1;}
    nChannels: Word;        {指出波形数据的声道数; 单声道为 1, 立体声为 2}
    nSamplesPerSec: DWORD;  {指定采样频率(每秒的样本数)}
    nAvgBytesPerSec: DWORD; {指定数据传输的传输速率(每秒的字节数)}
    nBlockAlign: Word;      {指定块对齐(每个样本的字节数), 块对齐是数据的最小单位}
    wBitsPerSample: Word;   {采样大小(字节), 每个样本的量化位数}
    cbSize: Word;           {附加信息的字节大小}
    end;
    {16 位立体声 PCM 的块对齐是 4 字节(每个样本2字节, 2个通道)}
    */
    
    WAVEFORMATEX waveFormatEx;  
    waveFormatEx.cbSize=0;  
    waveFormatEx.nAvgBytesPerSec=16000;  
    waveFormatEx.nBlockAlign=2;  
    waveFormatEx.nChannels=1;  
    waveFormatEx.nSamplesPerSec=44100;  
    waveFormatEx.wBitsPerSample=16;  
    waveFormatEx.wFormatTag=WAVE_FORMAT_PCM;     
    
    m_Opened = true;
    
    long sta;




     //打开播音设备并开始播音  
     int i;
     for(i=0;i < MAXWAVEBLOCK; i++)  
      WaveHdrOut[i].lpData=WaveOutBuffer[i];//设置播音数据内存区  
     hWaveOut = NULL;  
     if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormatEx, (unsigned long)waveOutProc, 0L, CALLBACK_FUNCTION))   
     {//播音设备打开失败，播音设备被其他程序占用  
      P("fail");
      return 1;  
     }  



    hWaveIn = NULL;  
    if(sta=waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormatEx, (unsigned long)waveInProc, 1L, CALLBACK_FUNCTION))   
    {//录音设备打开失败，可能声卡不支持全双工。  
         //ClosePhone();  
          return 2;  
    }  
   
  

   for(i=0;i < MAXWAVEBLOCK; i++)  
   {//增加录音内存  
           WaveHdrIn[i].lpData=WaveInBuffer[i];  
           WaveHdrIn[i].dwBufferLength=MAXWAVEBUFFER;  
           WaveHdrIn[i].dwFlags=0;  
           WaveHdrIn[i].dwUser=0; 
           //prepares a buffer for waveform-audio input 
           if(waveInPrepareHeader(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))  
           {
               return 3;  
           }   
           //sends an input buffer to the given waveform-audio input device. 
           //When the buffer is filled, the application is notified.
           if(waveInAddBuffer(hWaveIn,&WaveHdrIn[i],sizeof(WAVEHDR)))  
           {
                                                                      
               return 4;  
           }  
   }  
   
   if(waveInStart(hWaveIn))  
   {//无法开始录音  
       //ClosePhone();  
       return 5;  
   }  


   P("success!");

  

   
   int sinCount=0;

   //gen single
   for(i=1;i<nplay;i+=2)
   {      
          short int tmp = sin((double(sinCount)))*32767;
          a[i-1]=tmp&0xFF;
          a[i] = (tmp&0xFF00)>>8;
          sinCount++;
   }

   
   all = new char[nplay*genNum];
   for(int i=0;i<genNum;i++)
           memcpy(all+nplay*i,a,nplay);

    hThread1 = (HANDLE)_beginthreadex(NULL, 0, &UDPThread, (void *)pMsg1, 0, &uiThreadId1); //socket 
    hThread2 = (HANDLE)_beginthreadex(NULL, 0, &HardwareThread, (void *)pMsg2, 0, &uiThreadId2); //write hardware


    //等待线程结束 
    DWORD dwRet = WaitForSingleObject(hThread1,INFINITE); 
    if ( dwRet == WAIT_TIMEOUT ) 
    { 
        TerminateThread(hThread1,0); 
    } 
    dwRet = WaitForSingleObject(hThread2,INFINITE);
    if ( dwRet == WAIT_TIMEOUT ) 
    { 
        TerminateThread(hThread2,0); 
    } 
 
    //关闭线程句柄，释放资源 
    ::CloseHandle(hThread1); 
    ::CloseHandle(hThread2); 
    ::CloseHandle(m_mutex); 
    
 
  
  /*
  while(1)
  { 

  }
  */
    system("PAUSE");
    return EXIT_SUCCESS;
}
