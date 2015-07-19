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

SOCKET socket_out_socket;//��
struct sockaddr_in socket_out_addr;//
SOCKET socket_in_socket;
struct sockaddr_in socket_in_addr;
list<myBuffer*> bufList;


//global
HWAVEOUT hWaveOut;
HWAVEIN hWaveIn;  

bool m_Opened;
WAVEHDR WaveHdrOut[MAXWAVEBLOCK];  
//�ֲ�����  ?
static WAVEHDR WaveHdrIn[MAXWAVEBLOCK];  
//static char WaveOutBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//���沥���������ڴ��  
static char WaveInBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//��¼¼���������ڴ��  
static char WaveOutBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//���沥���������ڴ��  


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
 {//�жϻص�����  
 case WOM_OPEN:  
  //�����豸������ʱ���������¼�  
  P("o-open");
  //MessageBox(NULL,"Open success",NULL,MB_OK);  
  break;  
 case WOM_CLOSE:  
  //�����豸�����ر�ʱ���������¼�  
  //MessageBox(NULL,"Close success",NULL,MB_OK);  
  P("o-close");
  break;  
 case WOM_DONE:  
  //�ڴ沥�����ʱ���������¼�  
  if(!waveOutUnprepareHeader(hwo, dwParam1, sizeof(WAVEHDR)))  
  {  
   //delete dwParam1->lpData;//ɾ�����������ڴ�  
   //delete dwParam1;//ɾ���ṹ�����ڴ�  
   //m_pChange->DelDataBlock();//�����ڴ��������� 
   
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
    {//�ܲ������ݿ����Ҫ��   
        //char* buff1 = new char[RecieveLen];//���벥�������ڴ�   
        memcpy(WaveHdrOut[CurrentBlock].lpData, buf, RecieveLen);//������������   
   
        //WaveHdrOut = new WAVEHDR;//����ṹ�ڴ�   
        //Ϊ�ṹ��ֵ   
        WaveHdrOut[CurrentBlock].dwBufferLength=RecieveLen;   
        WaveHdrOut[CurrentBlock].dwFlags=0;   
        WaveHdrOut[CurrentBlock].dwUser=0;   
        //׼���������ڴ��   
        waveOutPrepareHeader(hWaveOut, &WaveHdrOut[CurrentBlock], sizeof(WAVEHDR));   
        //���Ÿ���������   
        waveOutWrite(hWaveOut, &WaveHdrOut[CurrentBlock], sizeof(WAVEHDR));   
        DataBlock++;   
        CurrentBlock = (CurrentBlock+1) % MAXWAVEBLOCK;   
        MaxDataBlock = MAXWAVEBLOCK;   
    }else   
    {//���ڴ����࣬��Ҫ���٣���2�����ڴ��   
     //�����ñ�֤�������Ϊ MAXWAVEBLOCK��������Ϊ2��   
     //�ڵ�ǰ�����£����������ʱΪ 0.5�루����������ʱ����   
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
 {//�жϻص�����  
 case WIM_OPEN:  
  //¼���豸������ʱ���������¼�  
   P("Opened");
  //MessageBox(NULL,"Open success", "��ʾ", MB_OK);  
  break;  
 case WIM_CLOSE:  
  //¼���豸�����ر�ʱ���������¼�  
  P("Closed");
  //MessageBox(NULL,"Close success", "��ʾ", MB_OK);  
  break;  
 case WIM_DATA:  
  //¼���豸��������ʱ���������¼�  
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
   //�����ͷ��ڴ档�е������Ȼ�������ݣ��������ͷ��ڴ棬  
   //��ʹ��ͬһ��WAVEHDR���Ӷ���ڴ��ʱ��������������  
   memcpy(buffer, dwParam1->lpData , MAXWAVEBUFFER);//���Ʒ�������  
   buf_len = dwParam1->dwBytesRecorded;//��¼¼�����ݳ���  
   
   /*The lpData, dwBufferLength, and dwFlags members must be set before
    calling the waveInPrepareHeader or waveOutPrepareHeader function. 
    (For either function, the dwFlags member must be set to zero.)
    */
   dwParam1->dwUser=0;//�ָ�����  
   dwParam1->dwBufferLength=MAXWAVEBUFFER;  
   dwParam1->dwFlags=0;  
   if(m_Opened)  
   {//¼�����ڼ���  
     //P("flow coming");
     result=waveInPrepareHeader(hwi,dwParam1,sizeof(WAVEHDR));  
     assert(!result);
     result=waveInAddBuffer(hwi,dwParam1,sizeof(WAVEHDR));  
     assert(!result);
   }  
  }  
  if(buf_len > 0 && buf_len <= MAXWAVEBUFFER)  
  {//��¼������  
      //P("has data");
      //cout<<"in";
      socket_send(buffer,buf_len);
      //dump(buffer,buf_len);
      
   //����¼������  
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
  socket_out_addr.sin_addr.s_addr=inet_addr("127.0.0.1"); ///server�ĵ�ַ 
  socket_out_addr.sin_port=htons(1234); ///server�ļ����˿�
  socket_out_socket=socket(AF_INET,SOCK_DGRAM,0);

}






//�̺߳��� 
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
    socket_in_addr.sin_port=htons(1500); ///�����˿�
    socket_in_addr.sin_addr.s_addr=inet_addr("127.0.0.1");; ///����
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



//�̺߳��� 
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
    TWaveFormatEx �ṹ:
    TWaveFormatEx = packed record
    wFormatTag: Word;       {ָ����ʽ����; Ĭ�� WAVE_FORMAT_PCM = 1;}
    nChannels: Word;        {ָ���������ݵ�������; ������Ϊ 1, ������Ϊ 2}
    nSamplesPerSec: DWORD;  {ָ������Ƶ��(ÿ���������)}
    nAvgBytesPerSec: DWORD; {ָ�����ݴ���Ĵ�������(ÿ����ֽ���)}
    nBlockAlign: Word;      {ָ�������(ÿ���������ֽ���), ����������ݵ���С��λ}
    wBitsPerSample: Word;   {������С(�ֽ�), ÿ������������λ��}
    cbSize: Word;           {������Ϣ���ֽڴ�С}
    end;
    {16 λ������ PCM �Ŀ������ 4 �ֽ�(ÿ������2�ֽ�, 2��ͨ��)}
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




     //�򿪲����豸����ʼ����  
     int i;
     for(i=0;i < MAXWAVEBLOCK; i++)  
      WaveHdrOut[i].lpData=WaveOutBuffer[i];//���ò��������ڴ���  
     hWaveOut = NULL;  
     if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormatEx, (unsigned long)waveOutProc, 0L, CALLBACK_FUNCTION))   
     {//�����豸��ʧ�ܣ������豸����������ռ��  
      P("fail");
      return 1;  
     }  



    hWaveIn = NULL;  
    if(sta=waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormatEx, (unsigned long)waveInProc, 1L, CALLBACK_FUNCTION))   
    {//¼���豸��ʧ�ܣ�����������֧��ȫ˫����  
         //ClosePhone();  
          return 2;  
    }  
   
  

   for(i=0;i < MAXWAVEBLOCK; i++)  
   {//����¼���ڴ�  
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
   {//�޷���ʼ¼��  
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


    //�ȴ��߳̽��� 
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
 
    //�ر��߳̾�����ͷ���Դ 
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
