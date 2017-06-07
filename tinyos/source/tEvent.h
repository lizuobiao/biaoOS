#ifndef TEVENT_H
#define TEVENT_H

#include "tConfig.h"
#include "tLib.h"
#include "tTask.h"

// Event类型
typedef enum  _tEventType {   
    tEventTypeUnknown   = (0 << 16), 				// 未知类型
    tEventTypeSem   	= (1 << 16), 				// 信号量类型
    tEventTypeMbox  	= (2 << 16), 				// 邮箱类型
	tEventTypeMemBlock  = (3 << 16),				// 存储块类型
	tEventTypeFlagGroup = (4 << 16),				// 事件标志组
	tEventTypeMutex     = (5 << 16),				// 互斥信号量类型
 }tEventType;

// Event控制结构
typedef struct _tEvent {
    tEventType type;						// Event类型

    tList waitList;							// 任务等待列表
}tEvent;

void tEventInit (tEvent * event, tEventType type);
void tEventWait (tEvent * event, tTask * task, void * msg, uint32_t state, uint32_t timeout);
tTask * tEventWakeUp (tEvent * event, void * msg, uint32_t result);
void tEventWakeUpTask (tEvent * event, tTask * task, void * msg, uint32_t result);
void tEventRemoveTask (tTask * task, void * msg, uint32_t result);

uint32_t tEventRemoveAll (tEvent * event, void * msg, uint32_t result);
uint32_t tEventWaitCount (tEvent * event);
	
#endif /* TEVENT_H */



