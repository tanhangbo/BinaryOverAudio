
class myBuffer{
  
public:

       myBuffer(int,char*);       
       myBuffer();
       unsigned char* getBuf(int *);  
       ~myBuffer();
       myBuffer& operator = (const myBuffer&);   
       void copyFrom(myBuffer);
private:
       int dataLen;
       unsigned char *p;

};
