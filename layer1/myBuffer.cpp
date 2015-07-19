#include "myBuffer.h"
#include "memory.h"
#include "stdio.h"
unsigned char* myBuffer::getBuf(int *len){
      *len = dataLen;
      return p;
}


myBuffer::myBuffer(int len,char *buf)
{
    p = new unsigned char[len];
    memcpy(p,buf,len);
    dataLen = len;
}
myBuffer::myBuffer()
{
   p=NULL;
   dataLen=0;
}

myBuffer::~myBuffer()
{
    delete [] p;
}


void myBuffer::copyFrom(myBuffer RightSides)
{
    dataLen=RightSides.dataLen;
    if(p == NULL)
         p =  new unsigned char[dataLen];
    else
    {
        delete []p;
        p =  new unsigned char[dataLen];  
    }   
    memcpy(p,RightSides.p,dataLen);    
}

//¸³Öµ²Ù×÷·ûÖØÔØ
myBuffer & myBuffer::operator = (const myBuffer& RightSides)
{         
    dataLen=RightSides.dataLen;
    
    if(p == NULL)
         p =  new unsigned char[dataLen];
    else
    {
        delete []p;
        p =  new unsigned char[dataLen];
        
    }   
    memcpy(p,RightSides.p,dataLen); 
    return *this; 
}
