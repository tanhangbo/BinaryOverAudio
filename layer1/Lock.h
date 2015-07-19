#ifndef Lock_H 
#define Lock_H  
 
#include <windows.h>  
 
//锁接口类 
class IMyLock 
{ 
public: 
    virtual ~IMyLock() {}  
 
    virtual void Lock() const = 0; 
    virtual void Unlock() const = 0; 
};  
 
//互斥对象锁类 
class Mutex : public IMyLock 
{ 
public: 
    Mutex(); 
    ~Mutex();  
 
    virtual void Lock() const; 
    virtual void Unlock() const;  
 
private: 
    HANDLE m_mutex; 
};  
 
//锁 
class CLock 
{ 
public: 
    CLock(const IMyLock&); 
    ~CLock();  
 
private: 
    const IMyLock& m_lock; 
}; 
 
#endif 
 


