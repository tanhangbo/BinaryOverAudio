#ifndef GLOBAL_H
#define GLOBAL_H
#define RING_SIZE 1000000

#include <QMutex>
#include <assert.h>
#include <QList>
extern unsigned char  ringBuffer[];
extern int ringEnd;
extern int ringStart;
extern int used;
extern QMutex myMutex;
extern QList<unsigned char> recvList;

#endif // GLOBAL_H
