#ifndef TSEM_H
#define TSEM_H

#include "tConfig.h"
#include "tEvent.h"

// 信号量类型
typedef struct _tSem 
{
	// 事件控制块
	// 该结构被特意放到起始处，以实现tSem同时是一个tEvent的目的
	tEvent event;

	// 当前的计数
	uint32_t count;

	// 最大计数
	uint32_t maxCount;
}tSem;

// 信号量的信息类型
typedef struct _tSemInfo
{
	// 当前信号量的计数
    uint32_t count;

    // 信号量允许的最大计数
    uint32_t maxCount;

    // 当前等待的任务计数
    uint32_t taskCount;
}tSemInfo;


void tSemInit (tSem * sem, uint32_t startCount, uint32_t maxCount);
uint32_t tSemWait (tSem * sem, uint32_t waitTicks);
uint32_t tSemNoWaitGet (tSem * sem);
void tSemNotify (tSem * sem);
void tSemGetInfo (tSem * sem, tSemInfo * info);
uint32_t tSemDestroy (tSem * sem);
#endif
