#ifndef _TIMYOS_H
#define _TIMYOS_H

#include <stdint.h>
#include "tLib.h"
#include "tconfig.h"
#include "tTask.h"
#include "tEvent.h"
#include "tSem.h"
#include "tMBox.h"

typedef enum _tError {
    tErrorNoError = 0,                              // 没有错误
    tErrorTimeout,                                  // 等待超时
    tErrorResourceUnavaliable,                 		// 等待超时
	tErrorDel,	
	tErrorResourceFull,	
}tError;

extern tTask * currentTask;
extern tTask * nextTask;

uint32_t tTaskEnterCritical (void);
void tTaskExitCritical (uint32_t status);

void tTaskRunFirst (void); 
void tTaskSwitch (void);

tTask * tTaskHighestReady (void);

void tTaskSchedInit (void);
void tTaskSchedDisable (void);
void tTaskSchedEnable (void);

void tTaskSchedRdy (tTask * task);
void tTaskSchedUnRdy (tTask * task);
void tTaskSchedRemove (tTask * task);
void tTaskSched (void);

void tTimeTaskWait (tTask * task, uint32_t ticks);
void tTimeTaskWakeUp (tTask * task);
void tTimeTaskRemove (tTask * task);

void tTaskDelay (uint32_t delay);
void tTaskSystemTickHandler (void);

void tInitApp (void);
void tSetSysTickPeriod(uint32_t ms);

#endif

